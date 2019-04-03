//
// Created by igor on 17.02.19.
//

#include <visitor/llvm/compiler.h>
#include <visitor/llvm/type.h>
#include <parser/nodes/concrete.h>
#include <parser/type.h>
#include <parser/type.h>

#include <iostream>
#include <cstdio>
#include <cstdlib>

#include <llvm/IR/Verifier.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>

// todo const and some kind of implicit dereferencing
Visitor::LLVM::Compiler::Compiler(Parser::AST &ast): Base(), _ast(ast) {}

// Base
void Visitor::LLVM::Compiler::visit(const Parser::Nodes::Base &node) {
    throw std::runtime_error("Cannot compile base node!");
}

// End
// Program
void Visitor::LLVM::Compiler::visit(const Parser::Nodes::Program &node) {
    // todo maybe init module here ?

    // precompile function protos
    for(const auto& func_proto: _ast.iter_func_prot()) {
        func_proto.second->accept(*this);
    }
    for(const auto& func_def: _ast.iter_func_def()) {
        func_def.second->declaration->accept(*this);
    }

    node.accept_children(*this);
    std::cout << "Printing module\n\n\n";
    _module->print(llvm::outs(), nullptr);
}

// Top Level
// Function
void Visitor::LLVM::Compiler::visit(const Parser::Nodes::FunctionProt &node) {
    if(_functions.count(node.identifier->symbol) > 0) {
        return;
    }

    auto ret_type = Type::to_llvm(*node.type, _context);
    std::vector<llvm::Type*> args_types;
    for(const auto& arg: node.arg_list) {
        args_types.push_back(Type::to_llvm(*arg->type, _context));
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

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::FunctionDef &node) {
    auto func_name = node.declaration->identifier->symbol;
    auto func_w = _functions.find(func_name);

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

    node.body->accept(*this);
    if(func_w->second.func->type->identifier().symbol == Type::void_id) {
        _builder.CreateRetVoid();
    }
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::ReturnStmt &node) {
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
void Visitor::LLVM::Compiler::visit(const Parser::Nodes::CodeBlock &node) {
    _ret_value = nullptr;
    for(auto& ch: node.children()) {
        ch->accept(*this);
    }
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::WhileStmt &node) {
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

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::BreakStmt &node) {
    if(!_curr_loop_contr) {
        throw std::runtime_error("Cannot break outside of loop");
    }
    _builder.CreateBr(_curr_loop_contr);
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::ContinueStmt &node) {
    if(!_curr_loop) {
        throw std::runtime_error("Cannot continue outside of loop");
    }
    _builder.CreateBr(_curr_loop);
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::BlockStmt &node) {
    node.body->accept(*this);
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::VariableDecl &node) {
    if(auto v = _local_variables.find(node.identifier->symbol); v != _local_variables.end()) {
        throw std::runtime_error("Redeclaration of a variable! " + node.identifier->symbol);
    }
    auto var = create_local_var(*_builder.GetInsertBlock()->getParent(), node);
    if(node.init_expr) {
        node.init_expr->accept(*this); auto init = _ret_value;
        if(!init) {
            throw std::runtime_error("Could not compile variable init expr");
        }
//        if(init->getType() != var.llvm_alloca->getType()) {
//            init = cast(init, var.llvm_alloca);
//        }
        auto old_action = _ptr_action;
        _ptr_action = PtrAction::Store;
        perform_ptr_action(var.llvm_alloca, init);
        _ptr_action = old_action;
    }
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::IfStmt &node) {
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
void Visitor::LLVM::Compiler::visit(const Parser::Nodes::RelationalExpr &node) {
    node.lhs->accept(*this); auto lhs = _ret_value;
    node.rhs->accept(*this); auto rhs = _ret_value;
    //auto [lhs, rhs] = promote(t_lhs, t_rhs);

    if (lhs->getType()->isIntegerTy()) {
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
    if (lhs->getType()->isFloatingPointTy()) {
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
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::AssignmentExpr &node) {
    // todo for now only identifier
    // todo support for access and indexing operators
    auto old_action = _ptr_action;
    _ptr_action = PtrAction::Address;
    node.lhs->accept(*this); auto lhs = _ret_value;
    if (!lhs) {
        throw std::runtime_error("Could not compile lhs of an assignment");
    }
    _ptr_action = PtrAction::Load;
    node.rhs->accept(*this); auto rhs = _ret_value;
    if (!rhs) {
        throw std::runtime_error("Could not compile left side of the assignment");
    }
//    if(lhs->getType() != rhs->getType()) {
//        rhs = cast(rhs, lhs);
//    }
    _ptr_action = PtrAction::Store;
    perform_ptr_action(lhs, rhs);
    _ptr_action = old_action;
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::AdditiveExpr &node) {
    node.lhs->accept(*this); auto lhs = _ret_value;
    node.rhs->accept(*this); auto rhs = _ret_value;
//    auto [lhs, rhs] = promote(t_lhs, t_rhs);

    if (lhs->getType()->isIntegerTy()) {
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
    if (lhs->getType()->isFloatingPointTy()) {
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
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::MultiplicativeExpr &node) {
    node.lhs->accept(*this); auto lhs = _ret_value;
    node.rhs->accept(*this); auto rhs = _ret_value;
//    auto [lhs, rhs] = promote(tmp_lhs, tmp_rhs);

    if (lhs->getType()->isIntegerTy()) {
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
    if(lhs->getType()->isFloatingPointTy()) {
        // both floats
        switch (node.op.id) {
            case Lexer::Token::Id::Multiplication:
                _ret_value = _builder.CreateFMul(lhs, rhs, "__multmp");
                break;
            case Lexer::Token::Id::Division:
                _ret_value = _builder.CreateFDiv(lhs, rhs, "__multmp");
                break;
            default:
                throw std::runtime_error("Unexpected operator during addition operator");
        }
    }
}

// Cast
void Visitor::LLVM::Compiler::visit(const Parser::Nodes::CastExpr &node) {
    node.lhs->accept(*this); auto lhs = _ret_value;

    auto from_type = lhs->getType();
    auto to_type = Type::to_llvm(*node.type, _context);

    if(!llvm::CastInst::isCastable(from_type, to_type)) {
        throw std::runtime_error("Cannot cast types");
    }
    // true means they are signed
    auto cast = _builder.CreateCast(
            llvm::CastInst::getCastOpcode(
                    lhs, true,
                    to_type, true),
            lhs,
            to_type,
            "__cast_tmp");
    if(!cast) {
        throw std::runtime_error("Could not compile cast");
    }
    _ret_value = cast;
}

// Unary
void Visitor::LLVM::Compiler::visit(const Parser::Nodes::AddressAccessExpr &node) {
    auto old_action = _ptr_action;
    _ptr_action = PtrAction::Address;
    node.rhs->accept(*this);
    _ptr_action = old_action;
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::DereferenceExpr &node) {
    auto old_action = _ptr_action;
    _ptr_action = PtrAction::Load;
    node.rhs->accept(*this);
    _ptr_action = old_action;
    _ret_value = perform_ptr_action(_ret_value, nullptr, "__deref_val");
}

// Postfix
// Todo for now its just for the identifiers
// todo chained function calls on function pointers
void Visitor::LLVM::Compiler::visit(const Parser::Nodes::CallExpr &node) {
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

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::IndexExpr &node) {
    std::cout << "Indexing\n";
    auto old_action = _ptr_action;
    _ptr_action = PtrAction::Address;
    node.lhs->accept(*this); auto lhs = _ret_value;
    _ptr_action = PtrAction::Load;
    node.index_expr->accept(*this); auto index = _ret_value;
    std::vector<llvm::Value*> gep_indexes{
        llvm::ConstantInt::get(_context, llvm::APInt(32, 0)),
        index};
    auto gep = _builder.CreateGEP(lhs, gep_indexes, "__gep_adr");

    _ptr_action = old_action; // Retrieve old action here so we know if we want to load or just ptr
    _ret_value = perform_ptr_action(gep, nullptr, "__gep_val");
}

// Primary
void Visitor::LLVM::Compiler::visit(const Parser::Nodes::Identifier &node) {
    auto var = _local_variables.find(node.symbol);
    if(var == _local_variables.end()) {
        // possible function call, we pass the identifier
        // todo make it work somehow later
        _ret_symbol = node.symbol;
        _ret_value = nullptr;
        return;
    }
    llvm::Value* v = var->second.llvm_alloca;
    _ret_value = perform_ptr_action(v, nullptr, node.symbol);
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::ParenthesisExpr &node) {
    node.expr->accept(*this);
}


// Consts
void Visitor::LLVM::Compiler::visit(const Parser::Nodes::IntConstant &node) {
    // todo for now ints are just 32 bits
    _ret_value = llvm::ConstantInt::get(_context, llvm::APInt(32, uint32_t(node.value)));
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::StringConstant &node) {
    throw std::runtime_error("No string support yet");
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::FloatConstant &node) {
    // todo for now just doubles
    _ret_value = llvm::ConstantFP::get(_context, llvm::APFloat(node.value));
}




Visitor::LLVM::Compiler::VarWrapper& Visitor::LLVM::Compiler::create_local_var(
        llvm::Function &func,
        const Parser::Nodes::VariableDecl &node) {

    if(_local_variables.count(node.identifier->symbol) > 0) {
        throw std::runtime_error("Variable redeclaration " + node.identifier->symbol);
    }
    llvm::IRBuilder<> tmp_b(&func.getEntryBlock(), func.getEntryBlock().begin());
    auto alloca = tmp_b.CreateAlloca(
        Type::to_llvm(*node.type, _context),
        nullptr,
        node.identifier->symbol);
    _local_variables[node.identifier->symbol] = VarWrapper{&node, alloca};
    return _local_variables[node.identifier->symbol];

}

llvm::Value *Visitor::LLVM::Compiler::cast(llvm::Value *from, llvm::Value *to) {
    std::cout << "Cast: " << from->getName().str() << " to: " << to->getName().str() << "\n";
    auto to_type = strip_ptr_type(to);
    auto from_type = strip_ptr_type(from);
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
    std::cout << "Cast ended!\n";
    return cast;
}

std::tuple<llvm::Value*, llvm::Value*> Visitor::LLVM::Compiler::promote(llvm::Value *lhs, llvm::Value *rhs) {
    auto l_type = strip_ptr_type(lhs);
    auto r_type = strip_ptr_type(rhs);
    if (l_type->isIntegerTy() && r_type->isIntegerTy()) {
       if(Type::int_size(l_type) > Type::int_size(r_type)) {
            rhs = cast(rhs, lhs);
       } else {
            lhs = cast(lhs, rhs);
       }
    } else if (l_type->isFloatingPointTy() && r_type->isIntegerTy()) {
        rhs = cast(rhs, lhs);
    } else if (l_type->isIntegerTy() && r_type->isFloatingPointTy()) {
        lhs = cast(lhs, rhs);
    } else if(l_type->isFloatingPointTy() && r_type->isFloatingPointTy()) {
        if(Type::float_size(l_type) > Type::float_size(r_type)) {
            rhs = cast(rhs, lhs);
        } else {
            lhs = cast(lhs, rhs);
        }
    }
    return std::make_tuple(lhs, rhs);
}

llvm::Type *Visitor::LLVM::Compiler::strip_ptr_type(llvm::Value *v) {
    auto v_type = v->getType();
    if(auto type_tmp = llvm::dyn_cast<llvm::PointerType>(v_type); type_tmp) {
        v_type = type_tmp->getElementType();
    }
    return v_type;
}

llvm::Value *Visitor::LLVM::Compiler::perform_ptr_action(
    llvm::Value *ptr, llvm::Value *v,
    const std::string &load_s) {

    switch (_ptr_action) {
        case PtrAction::None:
            return nullptr;
        case PtrAction::Store:
            if(!v) {
                throw std::runtime_error("Store of a null value");
            }
            return _builder.CreateStore(v, ptr);
        case PtrAction::Load:
            std::cout << "ptr action is load as " << load_s << "\n";
            return _builder.CreateLoad(ptr, load_s);
        case PtrAction::Address:
            return ptr;
    }
}
