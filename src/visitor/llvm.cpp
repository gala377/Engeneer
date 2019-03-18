//
// Created by igor on 17.02.19.
//

#include <visitor/llvm.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>

#include <parser/nodes/concrete.h>
#include <parser/type.h>

#include <iostream>
#include <cstdio>
#include <cstdlib>

#include <llvm/IR/Verifier.h>
#include <llvm/ADT/STLExtras.h>
#include <parser/type.h>


// todo allocas only in the first block


// Base
void Visitor::LLVM::visit(const Parser::Nodes::Base &node) {
    throw std::runtime_error("Cannot compile base node!");
}


// End
// Program
void Visitor::LLVM::visit(const Parser::Nodes::Program &node) {
    init_type_handlers();
    // todo maybe init module here ?
    node.accept_children(*this);

    std::cout << "Printing module\n\n\n";
    _module->print(llvm::outs(), nullptr);
}


// Top Level
// Function
void Visitor::LLVM::visit(const Parser::Nodes::FunctionProt &node) {
    auto ret_type = to_llvm_type(*node.type);
    std::vector<llvm::Type*> args_types;
    for(const auto& arg: node.arg_list) {
        args_types.push_back(to_llvm_type(*arg->type));
    }

    llvm::FunctionType* func_t = llvm::FunctionType::get(
            ret_type,
            args_types,
            false);
    llvm::Function* func = llvm::Function::Create(
            func_t,
            llvm::Function::ExternalLinkage,
            node.identifier->symbol,
            _module.get());
    if(!func) {
        throw std::runtime_error("Could not compile function!");
    }
    unsigned int i = 0;
    for(auto& arg: func->args()) {
        arg.setName(node.arg_list[i++]->identifier->symbol);
    }

    _functions[node.identifier->symbol] = FuncProtWrapper{&node, func};
}

void Visitor::LLVM::visit(const Parser::Nodes::FunctionDef &node) {
    auto func_name = node.declaration->identifier->symbol;
    auto func_w = _functions.find(func_name);
    if(func_w == _functions.end()) {
        node.declaration->accept(*this);
        func_w = _functions.find(func_name);
    }

    auto llvm_func = func_w->second.llvm_func;
    if(!llvm_func->empty()) {
        throw std::runtime_error("Redefinition of func " + node.declaration->identifier->symbol);
    }

    auto basic_block = llvm::BasicBlock::Create(_context, "entry", llvm_func);
    _builder.SetInsertPoint(basic_block);
    _local_variables.clear();

    unsigned i = 0;
    for(auto& arg: llvm_func->args()) {
        auto var = create_local_var(*llvm_func, *func_w->second.func->arg_list[i]);
        _builder.CreateStore(&arg, var.llvm_alloca);
        ++i;
    }

    // todo actually should subclass codeblock to function body
    // todo no func body parsing for now
    node.body->accept(*this);

    if(func_w->second.func->type->identifier().symbol == "void") {
        _builder.CreateRetVoid();
    }
    if(_ret_value) {
        llvm::verifyFunction(*llvm_func);
    } else {
        llvm_func->eraseFromParent();
    }
}

void Visitor::LLVM::visit(const Parser::Nodes::ReturnStmt &node) {
    _ret_value = nullptr;
    if(node.expr) {
        node.expr->accept(*this);
        if(_ret_value) {
            _builder.CreateRet(_ret_value);
        }
    } else {
        _builder.CreateRetVoid();
    }
}

// Statement
void Visitor::LLVM::visit(const Parser::Nodes::CodeBlock &node) {
    _ret_value = nullptr;
    for(auto& ch: node.children()) {
        ch->accept(*this);
    }
}

void Visitor::LLVM::visit(const Parser::Nodes::WhileStmt &node) {
    auto func = _builder.GetInsertBlock()->getParent();
    auto old_loop = _curr_loop;
    auto old_loop_contr = _curr_loop_contr;

    auto header_block = llvm::BasicBlock::Create(_context, "__whilecond");
    auto loop_block = llvm::BasicBlock::Create(_context, "__while");
    auto merge_block = llvm::BasicBlock::Create(_context, "__whilecontr");

    _curr_loop = header_block;
    _curr_loop_contr = merge_block;

    _builder.CreateBr(header_block);
    // Condition
    header_block->insertInto(func);
    _builder.SetInsertPoint(header_block);
    node.cond->accept(*this); auto cond = _ret_value;
    _builder.CreateCondBr(cond, loop_block, merge_block);

    // Body
    loop_block->insertInto(func);
    _builder.SetInsertPoint(loop_block);
    node.body->accept(*this);
    _builder.CreateBr(header_block);

    // merge
    merge_block->insertInto(func);
    _builder.SetInsertPoint(merge_block);

    _curr_loop = old_loop;
    _curr_loop_contr = old_loop_contr;
}

void Visitor::LLVM::visit(const Parser::Nodes::BreakStmt &node) {
    if(!_curr_loop_contr) {
        throw std::runtime_error("Cannot break outside of loop");
    }
    _builder.CreateBr(_curr_loop_contr);
}

void Visitor::LLVM::visit(const Parser::Nodes::ContinueStmt &node) {
    if(!_curr_loop) {
        throw std::runtime_error("Cannot continue outside of loop");
    }
    _builder.CreateBr(_curr_loop);
}

void Visitor::LLVM::visit(const Parser::Nodes::BlockStmt &node) {
    node.body->accept(*this);
}

void Visitor::LLVM::visit(const Parser::Nodes::VariableDecl &node) {
    if(auto v = _local_variables.find(node.identifier->symbol); v != _local_variables.end()) {
        throw std::runtime_error("Redeclaration of a variable! " + node.identifier->symbol);
    }
    auto var = create_local_var(*_builder.GetInsertBlock()->getParent(), node);
    if(node.init_expr) {
        node.init_expr->accept(*this); auto init = _ret_value;
        if(!init) {
            throw std::runtime_error("Could not compile variable init expr");
        }
        if(init->getType() != var.llvm_alloca->getType()) {
            std::cout << "Implicit casting...\n";
            init = cast(init, var.llvm_alloca);
        }
        _builder.CreateStore(init, var.llvm_alloca);
    }
}

void Visitor::LLVM::visit(const Parser::Nodes::IfStmt &node) {
    node.cond->accept(*this); auto cond = _ret_value;
    auto func = _builder.GetInsertBlock()->getParent();
    auto then = llvm::BasicBlock::Create(_context, "__iftrue");

    llvm::BasicBlock* else_clause = nullptr;
    if (node.else_clause) {
        else_clause = llvm::BasicBlock::Create(_context, "__iffalse");
    }
    auto merge = llvm::BasicBlock::Create(_context, "__ifcontr");

    if(else_clause) {
        _builder.CreateCondBr(cond, then, else_clause);
    } else {
        _builder.CreateCondBr(cond, then, merge);
    }

    // then body
    then->insertInto(func);
    _builder.SetInsertPoint(then);
    node.body->accept(*this);
    _builder.CreateBr(merge);

    // else body
    if(else_clause) {
        else_clause->insertInto(func);
        _builder.SetInsertPoint(else_clause);
        node.else_clause->accept(*this);
        _builder.CreateBr(merge);
    }

    merge->insertInto(func);
    _builder.SetInsertPoint(merge);
}


// Expression
// Binary

// todo for now only signed cmp
// todo we dont now how to make unsigned ints
// todo in future versions we need to know the type of the lhs
// todo and the rhs to make appropriate cmp
void Visitor::LLVM::visit(const Parser::Nodes::RelationalExpr &node) {
    node.lhs->accept(*this); auto lhs = _ret_value;
    node.rhs->accept(*this); auto rhs = _ret_value;
    // todo float compares
    if (lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy()) {
            switch(node.op.id) {
            case Lexer::Token::Id::LessThan:
                _ret_value = _builder.CreateICmpSLT(lhs, rhs, "__cmptemp");
                break;
            case Lexer::Token::Id::GreaterThan:
                _ret_value = _builder.CreateICmpSGT(lhs, rhs, "__cmptemp");
                break;
            case Lexer::Token::Id::LessEq:
                _ret_value = _builder.CreateICmpSLE(lhs, rhs, "__cmptemp");
                break;
            case Lexer::Token::Id::GreaterEq:
                _ret_value = _builder.CreateICmpSGE(lhs, rhs, "__cmptemp");
                break;
            default:
                throw std::runtime_error("Unexpected operator during addition operator");
        }
    }
    if (lhs->getType()->isFloatingPointTy() && rhs->getType()->isFloatingPointTy()) {
        switch(node.op.id) {
            case Lexer::Token::Id::LessThan:
                _ret_value = _builder.CreateFCmpULT(lhs, rhs, "__cmptemp");
                break;
            case Lexer::Token::Id::GreaterThan:
                _ret_value = _builder.CreateFCmpUGT(lhs, rhs, "__cmptemp");
                break;
            case Lexer::Token::Id::LessEq:
                _ret_value = _builder.CreateFCmpULE(lhs, rhs, "__cmptemp");
                break;
            case Lexer::Token::Id::GreaterEq:
                _ret_value = _builder.CreateFCmpUGE(lhs, rhs, "__cmptemp");
                break;
            default:
                throw std::runtime_error("Unexpected operator during addition operator");
        }
    }
    // todo float int, int float and casting
}

void Visitor::LLVM::visit(const Parser::Nodes::AssignmentExpr &node) {
    // todo for now only identifier
    // todo support for access and indexing operators
    _skip_load = true;
    node.lhs->accept(*this); auto lhs = _ret_value;
    if (!lhs) {
        throw std::runtime_error("Could not compile lhs of an assignment");
    }
    _skip_load = false;
    node.rhs->accept(*this); auto rhs = _ret_value;
    if (!rhs) {
        throw std::runtime_error("Could not compile left side of the assignment");
    }
    if(lhs->getType() != rhs->getType()) {
        std::cout << "Types not the same, implicit casting..." << "\n";
        rhs = cast(rhs, lhs);
    }

    _builder.CreateStore(rhs, lhs);
}

void Visitor::LLVM::visit(const Parser::Nodes::AdditiveExpr &node) {
    node.lhs->accept(*this); auto lhs = _ret_value;
    node.rhs->accept(*this); auto rhs = _ret_value;
    // todo based on type create float add
    if (lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy()) {
        switch (node.op.id) {
            case Lexer::Token::Id::Plus:
                _ret_value = _builder.CreateAdd(lhs, rhs, "__addtmp");
                break;
            case Lexer::Token::Id::Minus:
                _ret_value = _builder.CreateSub(lhs, rhs, "__addtmp");
                break;
            default:
                throw std::runtime_error("Unexpected operator during addition operator");
        }
    }
    if (lhs->getType()->isFloatingPointTy() && rhs->getType()->isFloatingPointTy()) {
        switch (node.op.id) {
            case Lexer::Token::Id::Plus:
                _ret_value = _builder.CreateFAdd(lhs, rhs, "__addtmp");
                break;
            case Lexer::Token::Id::Minus:
                _ret_value = _builder.CreateFSub(lhs, rhs, "__addtmp");
                break;
            default:
                throw std::runtime_error("Unexpected operator during addition operator");
        }
    }
    // todo float int, int float and casting
}

void Visitor::LLVM::visit(const Parser::Nodes::MultiplicativeExpr &node) {
    node.lhs->accept(*this); auto lhs = _ret_value;
    node.rhs->accept(*this); auto rhs = _ret_value;
    if (lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy()) {
        // both integers
        switch (node.op.id) {
            case Lexer::Token::Id::Multiplication:
                _ret_value = _builder.CreateMul(lhs, rhs, "__multmp");
                break;
            case Lexer::Token::Id::Division:
                // todo for now its unsigned division, we can do signed division
                // but its kinda more comlpicated
                _ret_value = _builder.CreateUDiv(lhs, rhs, "__multmp");
                break;
            default:
                throw std::runtime_error("Unexpected operator during addition operator");
        }
    }
    if(lhs->getType()->isFloatingPointTy() && rhs->getType()->isFloatingPointTy()) {
        // both floats
        switch (node.op.id) {
            case Lexer::Token::Id::Multiplication:
                _ret_value = _builder.CreateFMul(lhs, rhs, "__multmp");
                break;
            case Lexer::Token::Id::Division:
                // todo for now its unsigned division, we can do signed division
                // but its kinda more comlpicated
                _ret_value = _builder.CreateFDiv(lhs, rhs, "__multmp");
                break;
            default:
                throw std::runtime_error("Unexpected operator during addition operator");
        }
    }
    // todo float int, int float and casting
}


// Unary

// Postfix

// Todo for now its just for the identifiers
// todo I have no idea how to do this for the chained calls
// todo as we need a function name
void Visitor::LLVM::visit(const Parser::Nodes::CallExpr &node) {
    //std::cout << "call expr";
    // todo for now we assume lhs can only be an identifier
    node.lhs->accept(*this); auto symbol = _ret_symbol;

    auto callee = _module->getFunction(symbol);
    if(!callee) {
        throw std::runtime_error("Use of undeclared function");
    }

    if(callee->arg_size() != node.args.size()) {
        throw std::runtime_error("Incorrect # arguments passed");
    }

    std::vector<llvm::Value*> args_v;
    for(uint32_t i = 0; i != node.args.size(); ++i) {
        node.args[i]->accept(*this); auto arg = _ret_value;
        args_v.push_back(arg);
        if(!args_v.back()) {
            throw std::runtime_error("Could not emit function call argument");
        }
    }

    if(callee->getReturnType() == llvm::Type::getVoidTy(_context)) {
        _builder.CreateCall(callee, args_v);
        _ret_value = nullptr;
    } else {
        _ret_value = _builder.CreateCall(callee, args_v, "__calltmp");
    }
}

// Primary
void Visitor::LLVM::visit(const Parser::Nodes::Identifier &node) {
    auto var = _local_variables.find(node.symbol);
    if(var == _local_variables.end()) {
        // possible function call, we pass the identifier
        // todo make it work somehow later
        _ret_symbol = node.symbol;
        _ret_value = nullptr;
        return;
    }
    llvm::Value* v = var->second.llvm_alloca;
    if(_skip_load) {
        _ret_value = v;
    } else {
        _ret_value = _builder.CreateLoad(v, node.symbol);
    }
}

void Visitor::LLVM::visit(const Parser::Nodes::ParenthesisExpr &node) {
    node.expr->accept(*this);
}


// Consts
void Visitor::LLVM::visit(const Parser::Nodes::IntConstant &node) {
    //std::cout << "ConstInt\n";
    // todo for now ints are just 32 bits
    _ret_value = llvm::ConstantInt::get(_context, llvm::APInt(32, uint32_t(node.value)));
}


void Visitor::LLVM::visit(const Parser::Nodes::StringConstant &node) {
    throw std::runtime_error("No string support yet");
}

void Visitor::LLVM::visit(const Parser::Nodes::FloatConstant &node) {
    // todo for now just doubles
    _ret_value = llvm::ConstantFP::get(_context, llvm::APFloat(node.value));
}




Visitor::LLVM::VarWrapper& Visitor::LLVM::create_local_var(
        llvm::Function &func,
        const Parser::Nodes::VariableDecl &node) {

    if(_local_variables.count(node.identifier->symbol) > 0) {
        throw std::runtime_error("Variable redeclaration " + node.identifier->symbol);
    }
    llvm::IRBuilder<> tmp_b(&func.getEntryBlock(), func.getEntryBlock().begin());
    auto alloca = tmp_b.CreateAlloca(
        to_llvm_type(*node.type),
        nullptr,
        node.identifier->symbol);
    _local_variables[node.identifier->symbol] = VarWrapper{&node, alloca};
    return _local_variables[node.identifier->symbol];

}

llvm::Value *Visitor::LLVM::cast(llvm::Value *from, llvm::Value *to) {
    auto to_type = to->getType();
    if(llvm::dyn_cast<llvm::AllocaInst>(to) || llvm::dyn_cast<llvm::StoreInst>(to)) {
        auto type_tmp = llvm::dyn_cast<llvm::PointerType>(to_type);
        to_type = type_tmp->getElementType();
    }

    auto from_type =  from->getType();
    if(llvm::dyn_cast<llvm::AllocaInst>(from)) {
        auto type_tmp = llvm::dyn_cast<llvm::PointerType>(from_type);
        from_type = type_tmp->getElementType();
    }

    if(from_type == to_type) {
        return from;
    }
    if(!llvm::CastInst::isCastable(from_type, to_type)) {
        throw std::runtime_error("Cannot cast types");
    }
    // true means they are signed
    auto cast = _builder.CreateCast(
        llvm::CastInst::getCastOpcode(
            from, true,
            to_type, true),
        from,
        to_type,
        "__cast_tmp");
    if(!cast) {
        throw std::runtime_error("Could not compile cast");
    }
    return cast;
}

// todo const and some kind of implicit dereferencing
llvm::Type *Visitor::LLVM::to_llvm_type(const Parser::Types::BaseType &type) {
    if(auto t = try_as_simple(type); t) {
        return t;
    }
    if(auto t = try_as_complex(type); t) {
        return t;
    }
    if(auto t = try_as_array(type); t) {
        return t;
    }
    throw std::runtime_error("Unknown type: " + type.identifier().symbol);
}

llvm::Type *Visitor::LLVM::try_as_simple(const Parser::Types::BaseType &type) {
    try {
        const auto &primary = dynamic_cast<const Parser::Types::SimpleType&>(type);
        if(auto it = _type_handlers.find(primary.ident->symbol); it != _type_handlers.end()) {
            return it->second(type);
        }
        // todo possibly structure
        return nullptr;
    } catch (std::bad_cast&) {
        return nullptr;
    }
}

llvm::Type *Visitor::LLVM::try_as_array(const Parser::Types::BaseType &type) {
    try {
        const auto &array = dynamic_cast<const Parser::Types::ArrayType&>(type);
        return llvm::ArrayType::get(to_llvm_type(*array.underlying_type), array.size);
    } catch (std::bad_cast&) {
        return nullptr;
    }
}

llvm::Type *Visitor::LLVM::try_as_complex(const Parser::Types::BaseType &type) {
    try {
        const auto &complex = dynamic_cast<const Parser::Types::ComplexType&>(type);
        if(complex.is_ptr) {
            return llvm::PointerType::get(to_llvm_type(*complex.underlying_type), 0);
        }
    } catch (std::bad_cast&) {
        return nullptr;
    }
}

// todo no uints
void Visitor::LLVM::init_type_handlers() {
    _type_handlers["void"] = [this](const Parser::Types::BaseType& type) -> llvm::Type* {
        return llvm::Type::getVoidTy(_context);
    };
    _type_handlers["bool"] = [this](const Parser::Types::BaseType& type) -> llvm::Type* {
        return llvm::Type::getInt1Ty(_context);
    };
    _type_handlers["byte"] = [this](const Parser::Types::BaseType& type) -> llvm::Type* {
        return llvm::Type::getInt8Ty(_context);
    };
    _type_handlers["int8"] = [this](const Parser::Types::BaseType& type) -> llvm::Type* {
        return llvm::Type::getInt8Ty(_context);
    };
    _type_handlers["int16"] = [this](const Parser::Types::BaseType& type) -> llvm::Type* {
        return llvm::Type::getInt16Ty(_context);
    };
    _type_handlers["int32"] = [this](const Parser::Types::BaseType& type) -> llvm::Type* {
        return llvm::Type::getInt32Ty(_context);
    };
    _type_handlers["int64"] = [this](const Parser::Types::BaseType& type) -> llvm::Type* {
        return llvm::Type::getInt64Ty(_context);
    };
    _type_handlers["float32"] = [this](const Parser::Types::BaseType& type) -> llvm::Type* {
        return llvm::Type::getFloatTy(_context);
    };
    _type_handlers["float64"] = [this](const Parser::Types::BaseType& type) -> llvm::Type* {
        return llvm::Type::getDoubleTy(_context);
    };
    _type_handlers["float128"] = [this](const Parser::Types::BaseType& type) -> llvm::Type* {
        return llvm::Type::getFP128Ty(_context);
    };
}




