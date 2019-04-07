//
// Created by igor on 17.02.19.
//

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
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

    for(const auto& struct_def: _ast.iter_struct_decl()) {
        auto& s = declare_opaque(*struct_def.second);
        _curr_struct = &s;
        for(const auto& method: s.str->methods) {
            if(auto func = dynamic_cast<Parser::Nodes::FunctionDef*>(method.get()); func) {
                func->declaration->accept(*this);
                continue;
            }
            method->accept(*this);
        }
        _curr_struct = nullptr;
    }
    for(const auto& func_proto: _ast.iter_func_prot()) {
        func_proto.second->accept(*this);
    }
    for(const auto& func_def: _ast.iter_func_def()) {
        func_def.second->declaration->accept(*this);
    }


    node.accept_children(*this);
    std::cerr << "Printing module\n\n\n";
    _module->print(llvm::outs(), nullptr);
}

// Top Level
// Function
void Visitor::LLVM::Compiler::visit(const Parser::Nodes::FunctionProt &node) {
    if(_functions.count(node.identifier->symbol) > 0) {
        return;
    }
    auto ret_type = Type::to_llvm(*node.type, _context, _structs);
    std::vector<llvm::Type*> args_types;
    if(_curr_struct) {
        // its a method, we add this ptr
        args_types.push_back(llvm::PointerType::get(_curr_struct->llvm_str, 0));
    }
    for(const auto& arg: node.arg_list) {
        args_types.push_back(Type::to_llvm(*arg->type, _context, _structs));
    }
    
    auto identifier = node.identifier->symbol;
    if(_curr_struct) {
        identifier = meth_identifier(identifier);
    }
    llvm::FunctionType* func_t = llvm::FunctionType::get(
            ret_type,
            args_types,
            false);
    llvm::Function* func = llvm::Function::Create(
            func_t,
            llvm::Function::ExternalLinkage,
            identifier,
            _module.get());
    if(!func) {
        throw std::runtime_error("Could not compile function!");
    }
    unsigned int i = 0;
    bool this_id_set = false;
    for(auto& arg: func->args()) {
        if(_curr_struct && !this_id_set) {
            arg.setName(_this_identifier);
            this_id_set = true;
            continue;
        }
        arg.setName(node.arg_list[i++]->identifier->symbol);
    }
    _functions[identifier] = FuncProtWrapper{&node, func};
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::FunctionDef &node) {
    auto func_name = node.declaration->identifier->symbol;
    if(_curr_struct) {
        func_name = meth_identifier(func_name);
    }
    auto func_w = _functions.find(func_name);

    auto llvm_func = func_w->second.llvm_func;
    if(!llvm_func->empty()) {
        throw std::runtime_error("Redefinition of func " + func_name);
    }

    auto basic_block = llvm::BasicBlock::Create(_context, "entry", llvm_func);
    _builder.SetInsertPoint(basic_block);
    _local_variables.clear();

    unsigned i = 0;
    bool this_arg_set = false;
    for(auto& arg: llvm_func->args()) {
        if(_curr_struct && !this_arg_set) {
            this_arg_set = true;
            auto var = create_local_var(
                *llvm_func,
                _this_identifier,
                llvm::PointerType::get(_curr_struct->llvm_str, 0));
            _builder.CreateStore(&arg, var.llvm_alloca);
            continue;
        }
        auto var = create_local_var(*llvm_func, *func_w->second.func->arg_list[i]);
        _builder.CreateStore(&arg, var.llvm_alloca);
        ++i;
    }

    node.body->accept(*this);
    if(func_w->second.func->type->identifier().symbol == Type::void_id) {
        _builder.CreateRetVoid();
    }
}

Visitor::LLVM::Compiler::StructWrapper& Visitor::LLVM::Compiler::declare_opaque(const Parser::Nodes::StructDecl &node) {
    if(auto it = _structs.find(node.identifier->symbol); it != _structs.end()) {
        return it->second;
    }
    auto struct_type = llvm::StructType::create(_context, node.identifier->symbol);
    _structs[node.identifier->symbol] = StructWrapper{ &node, struct_type };
    return _structs[node.identifier->symbol];
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::StructDecl &node) {
    auto& s = declare_opaque(node);
    std::vector<llvm::Type*> fields;
    for(auto& field: node.members) {
        fields.emplace_back(Type::to_llvm(*(field->type), _context, _structs));
    }
    s.llvm_str->setBody(fields, false);
    
    // set compiling struct body context;
    _curr_struct = &s;
    for(auto& method: node.methods) {
        s.methods[method->ident().symbol] = compile_method(s.str, method.get());
    }
    _curr_struct = nullptr;
}

llvm::Function* Visitor::LLVM::Compiler::compile_method(
        const Parser::Nodes::StructDecl* str,
        const Parser::Nodes::FunctionDecl* meth) {
    meth->accept(*this);
    return _module->getFunction(meth_identifier(meth->ident().symbol));
}

std::string Visitor::LLVM::Compiler::meth_identifier(const std::string& m_name) {
    return "__" + _curr_struct->str->identifier->symbol + "__meth__" + m_name;
}

std::string Visitor::LLVM::Compiler::meth_identifier(const std::string& s_name, const std::string& m_name) {
    return "__" + s_name + "__meth__" + m_name;
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

    _ptr_action = PtrAction::Store;
    perform_ptr_action(lhs, rhs);
    _ptr_action = old_action;
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::AdditiveExpr &node) {
    node.lhs->accept(*this); auto lhs = _ret_value;
    node.rhs->accept(*this); auto rhs = _ret_value;

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
    auto to_type = Type::to_llvm(*node.type, _context, _structs);

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
void Visitor::LLVM::Compiler::visit(const Parser::Nodes::CallExpr &node) {
    // todo does it work for function ptrs and functions?
    auto old_action = _ptr_action;
    _ptr_action = PtrAction::Load;
    node.lhs->accept(*this); auto lhs = _ret_value;
    _ptr_action = old_action;

    auto func_ptr_t = llvm::dyn_cast<llvm::PointerType>(lhs->getType());
    if(!func_ptr_t) {
        std::cerr << lhs->getName().str() << " is not a Func ptr type\n Type is ";
        lhs->getType()->print(llvm::outs(), true, false);
    }
    auto func_t = llvm::dyn_cast<llvm::FunctionType>(func_ptr_t->getElementType());
    if(!func_t->isFunctionTy()) {
        std::cerr << lhs->getName().str() << "is not a Func type\n Type is ";
        func_ptr_t->getElementType()->print(llvm::outs(), true, false);
    }
    if(!func_t) {
        throw std::runtime_error("LHS of call expression is expected to be a function ptr");
    }
    if(func_t->params().size()!= node.args.size()) {
        throw std::runtime_error("Incorrect number of arguments passed");
    }
    std::vector<llvm::Value*> args_v;
    for(uint32_t i = 0; i != node.args.size(); ++i) {
        node.args[i]->accept(*this); auto arg = _ret_value;
        args_v.push_back(arg);
        if(!args_v.back()) {
            throw std::runtime_error("Could not emit function call argument");
        }
    }
    if(func_t->getReturnType() == llvm::Type::getVoidTy(_context)) {
        _builder.CreateCall(lhs, args_v);
        _ret_value = nullptr;
    } else {
        _ret_value = _builder.CreateCall(lhs, args_v, "__calltmp");
    }
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::IndexExpr &node) {
    std::cerr << "Indexing\n";
    auto old_action = _ptr_action;
    _ptr_action = PtrAction::Address;
    node.lhs->accept(*this); auto lhs = _ret_value;
    _ptr_action = PtrAction::Load;
    if(strip_ptr_type(lhs)->isPointerTy()) {
        // pointer to index type
        // we can implicit deref it
        lhs = perform_ptr_action(lhs, nullptr, "__impl_deref_index");
    }
    node.index_expr->accept(*this); auto index = _ret_value;
    std::vector<llvm::Value*> gep_indexes{
        llvm::ConstantInt::get(_context, llvm::APInt(32, 0)),
        index};
    auto gep = _builder.CreateGEP(lhs, gep_indexes, "__gep_adr");

    _ptr_action = old_action; // Retrieve old action here so we know if we want to load or just ptr
    _ret_value = perform_ptr_action(gep, nullptr, "__gep_val");
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::AccessExpr &node) {
    // get address of lhs for gep
    auto old_action = _ptr_action;
    _ptr_action = PtrAction::Address;
    node.lhs->accept(*this); auto lhs = _ret_value;
    // todo for now we assume only members
    // todo for methods me need to translate
    // todo its identifier to func ptr to call later
    Parser::Nodes::Identifier* ident = nullptr;
    if(ident = dynamic_cast<Parser::Nodes::Identifier*>(node.rhs.get()); ident == nullptr) {
        throw std::runtime_error("Expected identifier for the access operator");
    }
    auto gep = access_struct_field(lhs, ident->symbol);
    _ptr_action = old_action; // Retrieve old action here so we know if we want to load or just ptr
    // todo but here is a problem, we really dont want to load function
    // pointers if we return a method  
    _ret_value = perform_ptr_action(gep, nullptr, "__gep_val");
}

llvm::Value* Visitor::LLVM::Compiler::access_struct_field(llvm::Value* str, const std::string& field_name) {
    // alloca ptr strip
    auto expr_type = strip_ptr_type(str);
    if(!(expr_type->isStructTy())) {
        // could be pointer to struct
        expr_type = strip_ptr_type(expr_type);
        if(!(expr_type->isStructTy())) {
            throw std::runtime_error("Access can only be done on structs");            
        }
        // implicit pointer dereference
        auto old_action = _ptr_action;
        _ptr_action = PtrAction::Load;
        str = perform_ptr_action(str, nullptr, "__impl_deref_struct_load");
        _ptr_action = old_action;
    }
    auto& s_wrapper = _structs[llvm::dyn_cast<llvm::StructType>(expr_type)->getName()];
    std::int32_t gep_index = s_wrapper.member_index(field_name);
    std::vector<llvm::Value*> gep_indexes{
        llvm::ConstantInt::get(_context, llvm::APInt(32, 0)),
        llvm::ConstantInt::get(_context, llvm::APInt(32, (uint64_t)gep_index))};
    auto gep = _builder.CreateGEP(str, gep_indexes, "__gep_adr");
    return gep;
}


// Primary
void Visitor::LLVM::Compiler::visit(const Parser::Nodes::Identifier &node) {
    auto var = _local_variables.find(node.symbol);
    if(var == _local_variables.end()) {
        auto func = _module->getFunction(node.symbol);
        if(func == nullptr && _curr_struct) {
            auto str_this = _local_variables[_this_identifier];
            auto v = access_struct_field(str_this.llvm_alloca, node.symbol);
            _ret_value = perform_ptr_action(v, nullptr, node.symbol);
            return;
        }
        _ret_value = func;
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
        Type::to_llvm(*node.type, _context, _structs),
        nullptr,
        node.identifier->symbol);
    _local_variables[node.identifier->symbol] = VarWrapper{&node, alloca};
    return _local_variables[node.identifier->symbol];

}

Visitor::LLVM::Compiler::VarWrapper& Visitor::LLVM::Compiler::create_local_var(
            llvm::Function &func,
            const std::string& identifier,
            llvm::Type* type) {

    if(_local_variables.count(identifier) > 0) {
        throw std::runtime_error("Variable redeclaration " + identifier);
    }
    llvm::IRBuilder<> tmp_b(&func.getEntryBlock(), func.getEntryBlock().begin());
    auto alloca = tmp_b.CreateAlloca(
        type,
        nullptr,
        identifier);
    _local_variables[identifier] = VarWrapper{nullptr, alloca};
    return _local_variables[identifier];
}


llvm::Value *Visitor::LLVM::Compiler::cast(llvm::Value *from, llvm::Value *to) {
    std::cerr << "Cast: " << from->getName().str() << " to: " << to->getName().str() << "\n";
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
    std::cerr << "Cast ended!\n";
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

llvm::Type *Visitor::LLVM::Compiler::strip_ptr_type(llvm::Type *v) {
    auto v_type = v;
    if(auto type_tmp = llvm::dyn_cast<llvm::PointerType>(v_type); type_tmp) {
        v_type = type_tmp->getElementType();
    }
    return v_type;
}

llvm::Type *Visitor::LLVM::Compiler::strip_ptr_type(llvm::Value *v) {
    return strip_ptr_type(v->getType());
}

llvm::Value *Visitor::LLVM::Compiler::perform_ptr_action(
    llvm::Value *ptr, llvm::Value *v,
    const std::string &load_s) {

    switch (_ptr_action) {
        case PtrAction::None:
            std::cerr << "Ptr action is None\n";
            return nullptr;
        case PtrAction::Store:
            std::cerr << "Ptr action is store to " << ptr->getName().str() << "\n";
            if(!v) {
                throw std::runtime_error("Store of a null value");
            }
            return _builder.CreateStore(v, ptr);
        case PtrAction::Load:
            std::cerr << "ptr action is load as " << load_s << " from: " << ptr->getName().str() <<"\n";
            return _builder.CreateLoad(ptr, load_s);
        case PtrAction::Address:
            std::cerr << "ptr actions is address of " << ptr->getName().str() << "\n";
            return ptr;
    }
}


std::int32_t Visitor::LLVM::Compiler::StructWrapper::member_index(const std::string &name) const {
    for(std::uint32_t i = 0; i < str->members.size(); ++i) {
        if(name == str->members[i]->identifier->symbol) {
            return i;
        }
    }
    throw std::runtime_error("Unknown struct member " + name);
}


