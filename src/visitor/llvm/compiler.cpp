//
// Created by igor on 17.02.19.
//

#include <llvm/ADT/Optional.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Value.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>
#include <vector>
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
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Support/Casting.h>

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
    init_compile_target();
    // declare struct opaque
    for(const auto& struct_def: _ast.iter_struct_decl()) {
        auto& s = declare_opaque(*struct_def.second);
        _ctx.curr_struct = &s;
        for(const auto& method: s.str->methods) {
            // declare all its methods prototypes
            if(auto func = dynamic_cast<Parser::Nodes::FunctionDef*>(method.get()); func) {
                func->declaration->accept(*this);
                continue;
            }
            method->accept(*this);
        }
        _ctx.curr_struct = nullptr;
    }
    // the same for mem defs
    // todo refactor
    for(const auto& mem_def: _ast.iter_memory_decl()) {
        auto& s = declare_opaque(*mem_def.second);
        _ctx.curr_struct = &s;
        for(const auto& method: s.str->methods) {
            // declare all its methods prototypes
            if(auto func = dynamic_cast<Parser::Nodes::FunctionDef*>(method.get()); func) {
                func->declaration->accept(*this);
                continue;
            }
            method->accept(*this);
        }
        _ctx.curr_struct = nullptr;
    }
    // declare all function protos
    for(const auto& func_proto: _ast.iter_func_prot()) {
        func_proto.second->accept(*this);
    }
    for(const auto& func_def: _ast.iter_func_def()) {
        func_def.second->declaration->accept(*this);
    }
    // declare all struct bodies
    for(const auto& struct_def: _ast.iter_struct_decl()) {
        declare_body(*struct_def.second);
    }

    for(const auto& mem_def: _ast.iter_memory_decl()) {
        declare_body(*mem_def.second);
        add_memory_global_var(*mem_def.second);
    }

    for(const auto& var: _ast.iter_glob_var_decl()) {
        var.second->accept(*this);
    }

    // compile functions
    // compile methods
    node.accept_children(*this);
    _module->print(llvm::outs(), nullptr);
    emit_obj_code();
}


void Visitor::LLVM::Compiler::init_compile_target() {
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    
    auto target_triple = llvm::sys::getDefaultTargetTriple();
    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(target_triple, error);
    if(!target) {
        throw std::runtime_error(error);
    }
    auto cpu = "generic";
    auto features = "";
    llvm::TargetOptions opt;
    auto rm = llvm::Optional<llvm::Reloc::Model>();
    auto target_machine = target->createTargetMachine(
        target_triple,
        cpu,
        features,
        opt,
        rm);

    _module->setDataLayout(target_machine->createDataLayout());
    _module->setTargetTriple(target_triple);
    _target_machine = target_machine;
    
}

void Visitor::LLVM::Compiler::emit_obj_code() {
    auto filename = "output.o";
    std::error_code ec;
    llvm::raw_fd_ostream dest(filename, ec, llvm::sys::fs::OpenFlags::F_None);

    if(ec) {
        std::cerr << "Could not open file: " << ec.message() << "\n";
        throw std::runtime_error(ec.message());
    }

    llvm::legacy::PassManager pass;
    auto file_type = llvm::TargetMachine::CGFT_ObjectFile;
    
    if(_target_machine->addPassesToEmitFile(pass, dest, nullptr, file_type)) {
        throw std::runtime_error("Target machine can't emit a file of this type");
    }
    pass.run(*_module);
    dest.flush();
    delete _target_machine;
}

// Top Level
// Function
void Visitor::LLVM::Compiler::visit(const Parser::Nodes::FunctionProt &node) {
    if(_ctx.functions.count(node.identifier->symbol) > 0) {
        return;
    }
    auto ret_type = Type::to_llvm(*node.type, _context, _ctx.structs);
    std::vector<llvm::Type*> args_types;
    if(_ctx.curr_struct) {
        // its a method, we add this ptr
        args_types.push_back(llvm::PointerType::get(_ctx.curr_struct->llvm_str, 0));
    }
    for(const auto& arg: node.arg_list) {
        args_types.push_back(Type::to_llvm(*arg->type, _context, _ctx.structs));
    }
    
    auto identifier = node.identifier->symbol;
    if(_ctx.curr_struct) {
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
        if(_ctx.curr_struct && !this_id_set) {
            arg.setName(_this_identifier);
            this_id_set = true;
            continue;
        }
        arg.setName(node.arg_list[i++]->identifier->symbol);
    }
    _ctx.functions[identifier] = FuncProtWrapper{&node, func, _ctx.curr_struct != nullptr};
    if(_ctx.curr_struct) {
        _ctx.curr_struct->methods[node.identifier->symbol] = &_ctx.functions[identifier];
    }
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::FunctionDef &node) {
    auto func_name = node.declaration->identifier->symbol;
    if(_ctx.curr_struct) {
        func_name = meth_identifier(func_name);
    }
    auto func_w = _ctx.functions.find(func_name);

    auto llvm_func = func_w->second.llvm_func;
    if(!llvm_func->empty()) {
        throw std::runtime_error("Redefinition of func " + func_name);
    }

    auto basic_block = llvm::BasicBlock::Create(_context, "entry", llvm_func);
    _builder.SetInsertPoint(basic_block);
    _ctx.local_variables.clear();

    unsigned i = 0;
    bool this_arg_set = false;
    for(auto& arg: llvm_func->args()) {
        if(_ctx.curr_struct && !this_arg_set) {
            this_arg_set = true;
            auto var = create_local_var(
                *llvm_func,
                _this_identifier,
                llvm::PointerType::get(_ctx.curr_struct->llvm_str, 0));
            _builder.CreateStore(&arg, var.llvm_alloca);
            continue;
        }
        auto var = create_local_var(*llvm_func, *func_w->second.func->arg_list[i]);
        _builder.CreateStore(&arg, var.llvm_alloca);
        ++i;
    }
    //std::cerr << "In function " << func_name << "\n";
    if(func_name == "main") {
        add_memory_initializers();
    }
    node.body->accept(*this);
    if(func_w->second.func->type->identifier().symbol == Type::void_id) {
        _builder.CreateRetVoid();
    }
}

void Visitor::LLVM::Compiler::add_memory_initializers() {
    for(auto& s: _ctx.structs) {
        if(!s.second.is_memory) {
            continue;
        }
        if(auto meth = s.second.methods.find(_mem_init_meth_name); meth == s.second.methods.end()) {
            continue;
        } 
        auto& glob_var = _ctx.global_variables[mem_glob_identifier(s.second.str->identifier->symbol)];
        auto meth_name = meth_identifier(
            s.second.str->identifier->symbol,
            _mem_init_meth_name);
        auto lhs = _ctx.functions[meth_name].llvm_func;
        std::vector<llvm::Value*> args;
        args.push_back(glob_var.llvm_var);
        _builder.CreateCall(lhs, args);
    }
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::GlobVariableDecl &node) {
    if(auto it = _ctx.global_variables.find(node.identifier->symbol); it != _ctx.global_variables.end()) {
        return;
    }
    if(!node.init_expr) {
        throw std::runtime_error("Init expression of a global variable cannot be empty");
    }
    node.init_expr->accept(*this); auto init = _ctx.ret_value;
    auto const_init = llvm::dyn_cast<llvm::Constant>(init);
    if(!const_init) {
        throw std::runtime_error("Global variables can only be initialized with constant values");
    }
    auto glob_var = new llvm::GlobalVariable(
        *_module,
        Type::to_llvm(*node.type, _context, _ctx.structs),
        false, // if its const 
        llvm::GlobalValue::InternalLinkage,
        const_init,
        node.identifier->symbol);
    _ctx.global_variables[node.identifier->symbol] = {
        &node,
        glob_var,
    };
}

Visitor::LLVM::Compiler::StructWrapper& Visitor::LLVM::Compiler::declare_opaque(const Parser::Nodes::StructDecl &node) {
    if(auto it = _ctx.structs.find(node.identifier->symbol); it != _ctx.structs.end()) {
        return it->second;
    }
    auto struct_type = llvm::StructType::create(_context, node.identifier->symbol);
    _ctx.structs[node.identifier->symbol] = StructWrapper{ &node, struct_type };
    return _ctx.structs[node.identifier->symbol];
}

Visitor::LLVM::Compiler::StructWrapper& Visitor::LLVM::Compiler::declare_opaque(const Parser::Nodes::MemoryDecl &node) {
    auto& s = declare_opaque(static_cast<const Parser::Nodes::StructDecl&>(node));
    s.is_memory = true;
    return s;
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::StructDecl &node) {
    auto& s = declare_opaque(node);    
    // set compiling struct body context;
    _ctx.curr_struct = &s;
    for(auto& method: node.methods) {
        compile_method(s.str, method.get());
    }
    _ctx.curr_struct = nullptr;
}

Visitor::LLVM::Compiler::FuncProtWrapper* Visitor::LLVM::Compiler::compile_method(
        const Parser::Nodes::StructDecl* str,
        const Parser::Nodes::FunctionDecl* meth) {
    meth->accept(*this);
    return &(_ctx.functions[meth_identifier(meth->ident().symbol)]);
}
Visitor::LLVM::Compiler::StructWrapper& Visitor::LLVM::Compiler::declare_body(
        const Parser::Nodes::StructDecl& node) {
    auto& s = declare_opaque(node);
    std::vector<llvm::Type*> fields;
    for(auto& field: node.members) {
        fields.emplace_back(Type::to_llvm(*(field->type), _context, _ctx.structs));
    }
    s.llvm_str->setBody(fields, false);
    return s;       
}

std::string Visitor::LLVM::Compiler::meth_identifier(const std::string& m_name) {
    return "__" + _ctx.curr_struct->str->identifier->symbol + "__meth__" + m_name;
}

std::string Visitor::LLVM::Compiler::meth_identifier(const std::string& s_name, const std::string& m_name) {
    return "__" + s_name + "__meth__" + m_name;
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::MemoryDecl &node) {
    visit(static_cast<const Parser::Nodes::StructDecl&>(node));
}

Visitor::LLVM::Compiler::GlobalVarWrapper& Visitor::LLVM::Compiler::add_memory_global_var(const Parser::Nodes::MemoryDecl& node) {
    auto identifier = mem_glob_identifier(node.identifier->symbol);
    auto type = Type::to_llvm(
        Parser::Types::SimpleType(
            std::make_unique<Parser::Nodes::Identifier>(node.identifier->symbol)),
        _context,
        _ctx.structs);
    auto glob_var = new llvm::GlobalVariable(
        *_module,
        type,
        false, // if its const 
        llvm::GlobalValue::InternalLinkage,
        llvm::ConstantAggregateZero::get(type),
        identifier);
    _ctx.global_variables[identifier] = {
        nullptr,
        glob_var,
    };
    return _ctx.global_variables[identifier];
}


std::string Visitor::LLVM::Compiler::mem_glob_identifier(const std::string& m_name) {
    return "__memory__" + m_name;
}

// Statement
void Visitor::LLVM::Compiler::visit(const Parser::Nodes::CodeBlock &node) {
    _ctx.ret_value = nullptr;
    for(auto& ch: node.children()) {
        ch->accept(*this);
    }
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::WhileStmt &node) {
    auto func = _builder.GetInsertBlock()->getParent();
    auto old_loop = _ctx.loop;
    auto old_loop_contr = _ctx.loop_contr;

    auto header_block = llvm::BasicBlock::Create(_context, "__whilecond");
    auto loop_block = llvm::BasicBlock::Create(_context, "__while");
    auto merge_block = llvm::BasicBlock::Create(_context, "__whilecontr");

    _ctx.loop = header_block;
    _ctx.loop_contr = merge_block;

    _builder.CreateBr(header_block);
    // Condition
    header_block->insertInto(func);
    _builder.SetInsertPoint(header_block);
    node.cond->accept(*this); auto cond = _ctx.ret_value;
    _builder.CreateCondBr(cond, loop_block, merge_block);

    // Body
    loop_block->insertInto(func);
    _builder.SetInsertPoint(loop_block);
    node.body->accept(*this);
    _builder.CreateBr(header_block);

    // merge
    merge_block->insertInto(func);
    _builder.SetInsertPoint(merge_block);

    _ctx.loop = old_loop;
    _ctx.loop_contr = old_loop_contr;
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::BreakStmt &node) {
    if(!_ctx.loop_contr) {
        throw std::runtime_error("Cannot break outside of loop");
    }
    _builder.CreateBr(_ctx.loop_contr);
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::ContinueStmt &node) {
    if(!_ctx.loop) {
        throw std::runtime_error("Cannot continue outside of loop");
    }
    _builder.CreateBr(_ctx.loop);
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::BlockStmt &node) {
    node.body->accept(*this);
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::VariableDecl &node) {
    if(auto v = _ctx.local_variables.find(node.identifier->symbol); v != _ctx.local_variables.end()) {
        throw std::runtime_error("Redeclaration of a variable! " + node.identifier->symbol);
    }
    auto var = create_local_var(*_builder.GetInsertBlock()->getParent(), node);
    if(node.init_expr) {
        node.init_expr->accept(*this); auto init = _ctx.ret_value;
        if(!init) {
            throw std::runtime_error("Could not compile variable init expr");
        }
        auto old_action = _ctx.ptr_action;
        _ctx.ptr_action = PtrAction::Store;
        perform_ptr_action(var.llvm_alloca, init);
        _ctx.ptr_action = old_action;
    }
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::IfStmt &node) {
    node.cond->accept(*this); auto cond = _ctx.ret_value;
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
    _ctx.ret_value = nullptr;
    if(node.expr) {
        node.expr->accept(*this);
        if(_ctx.ret_value) {
            _builder.CreateRet(_ctx.ret_value);
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
    node.lhs->accept(*this); auto lhs = _ctx.ret_value;
    node.rhs->accept(*this); auto rhs = _ctx.ret_value;
    //auto [lhs, rhs] = promote(t_lhs, t_rhs);

    if (lhs->getType()->isIntegerTy()) {
            switch(node.op.id) {
            case Lexer::Token::Id::LessThan:
                _ctx.ret_value = _builder.CreateICmpSLT(lhs, rhs, "__cmptemp");
                break;
            case Lexer::Token::Id::GreaterThan:
                _ctx.ret_value = _builder.CreateICmpSGT(lhs, rhs, "__cmptemp");
                break;
            case Lexer::Token::Id::LessEq:
                _ctx.ret_value = _builder.CreateICmpSLE(lhs, rhs, "__cmptemp");
                break;
            case Lexer::Token::Id::GreaterEq:
                _ctx.ret_value = _builder.CreateICmpSGE(lhs, rhs, "__cmptemp");
                break;
            default:
                throw std::runtime_error("Unexpected operator during addition operator");
        }
    }
    if (lhs->getType()->isFloatingPointTy()) {
        switch(node.op.id) {
            case Lexer::Token::Id::LessThan:
                _ctx.ret_value = _builder.CreateFCmpULT(lhs, rhs, "__cmptemp");
                break;
            case Lexer::Token::Id::GreaterThan:
                _ctx.ret_value = _builder.CreateFCmpUGT(lhs, rhs, "__cmptemp");
                break;
            case Lexer::Token::Id::LessEq:
                _ctx.ret_value = _builder.CreateFCmpULE(lhs, rhs, "__cmptemp");
                break;
            case Lexer::Token::Id::GreaterEq:
                _ctx.ret_value = _builder.CreateFCmpUGE(lhs, rhs, "__cmptemp");
                break;
            default:
                throw std::runtime_error("Unexpected operator during addition operator");
        }
    }
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::AssignmentExpr &node) {
    auto old_action = _ctx.ptr_action;
    _ctx.ptr_action = PtrAction::Address;
    node.lhs->accept(*this); auto lhs = _ctx.ret_value;
    if (!lhs) {
        throw std::runtime_error("Could not compile lhs of an assignment");
    }
    _ctx.ptr_action = PtrAction::Load;
    node.rhs->accept(*this); auto rhs = _ctx.ret_value;
    if (!rhs) {
        throw std::runtime_error("Could not compile left side of the assignment");
    }

    _ctx.ptr_action = PtrAction::Store;
    perform_ptr_action(lhs, rhs);
    _ctx.ptr_action = old_action;
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::AdditiveExpr &node) {
    node.lhs->accept(*this); auto lhs = _ctx.ret_value;
    node.rhs->accept(*this); auto rhs = _ctx.ret_value;

    if (lhs->getType()->isIntegerTy()) {
        switch (node.op.id) {
            case Lexer::Token::Id::Plus:
                _ctx.ret_value = _builder.CreateAdd(lhs, rhs, "__addtmp");
                break;
            case Lexer::Token::Id::Minus:
                _ctx.ret_value = _builder.CreateSub(lhs, rhs, "__addtmp");
                break;
            default:
                throw std::runtime_error("Unexpected operator during addition operator");
        }
    }
    if (lhs->getType()->isFloatingPointTy()) {
        switch (node.op.id) {
            case Lexer::Token::Id::Plus:
                _ctx.ret_value = _builder.CreateFAdd(lhs, rhs, "__addtmp");
                break;
            case Lexer::Token::Id::Minus:
                _ctx.ret_value = _builder.CreateFSub(lhs, rhs, "__addtmp");
                break;
            default:
                throw std::runtime_error("Unexpected operator during addition operator");
        }
    }
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::MultiplicativeExpr &node) {
    node.lhs->accept(*this); auto lhs = _ctx.ret_value;
    node.rhs->accept(*this); auto rhs = _ctx.ret_value;

    if (lhs->getType()->isIntegerTy()) {
        // both integers
        switch (node.op.id) {
            case Lexer::Token::Id::Multiplication:
                _ctx.ret_value = _builder.CreateMul(lhs, rhs, "__multmp");
                break;
            case Lexer::Token::Id::Division:
                // todo for now its unsigned division, we can do signed division
                // but its kinda more comlpicated
                _ctx.ret_value = _builder.CreateUDiv(lhs, rhs, "__multmp");
                break;
            default:
                throw std::runtime_error("Unexpected operator during addition operator");
        }
    }
    if(lhs->getType()->isFloatingPointTy()) {
        // both floats
        switch (node.op.id) {
            case Lexer::Token::Id::Multiplication:
                _ctx.ret_value = _builder.CreateFMul(lhs, rhs, "__multmp");
                break;
            case Lexer::Token::Id::Division:
                _ctx.ret_value = _builder.CreateFDiv(lhs, rhs, "__multmp");
                break;
            default:
                throw std::runtime_error("Unexpected operator during addition operator");
        }
    }
}

// Cast
void Visitor::LLVM::Compiler::visit(const Parser::Nodes::CastExpr &node) {
    node.lhs->accept(*this); auto lhs = _ctx.ret_value;

    auto from_type = lhs->getType();
    auto to_type = Type::to_llvm(*node.type, _context, _ctx.structs);

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
    _ctx.ret_value = cast;
}

// Unary
void Visitor::LLVM::Compiler::visit(const Parser::Nodes::AddressAccessExpr &node) {
    auto old_action = _ctx.ptr_action;
    _ctx.ptr_action = PtrAction::Address;
    node.rhs->accept(*this);
    _ctx.ptr_action = old_action;
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::DereferenceExpr &node) {
    auto old_action = _ctx.ptr_action;
    _ctx.ptr_action = PtrAction::Load;
    node.rhs->accept(*this);
    _ctx.ptr_action = old_action;
    _ctx.ret_value = perform_ptr_action(_ctx.ret_value, nullptr, "__deref_val");
}

// Postfix
void Visitor::LLVM::Compiler::visit(const Parser::Nodes::CallExpr &node) {
    auto [lhs, this_inst, func] = compile_call_lhs(*node.lhs);

    auto func_ptr_t = llvm::dyn_cast<llvm::PointerType>(lhs->getType());
    //std::cerr << "CE: Casted to ptr\n";    
    auto func_t = llvm::dyn_cast<llvm::FunctionType>(func_ptr_t->getElementType());
    if(!func_t) {
        throw std::runtime_error("LHS of call expression is expected to be a function ptr");
    }
    //std::cerr << "Is method call?\n";
    auto is_meth_call = func->is_method && this_inst;
    std::uint32_t add_parameters = is_meth_call ? 1 : 0;
    if(func_t->params().size() != (node.args.size() + add_parameters)) {
        std::string mess{"Incorrect number of arguments passed. Expected: "};
        mess += std::to_string(func_t->params().size() - add_parameters);
        mess += " Got: " + std::to_string(node.args.size());
        throw std::runtime_error(mess.c_str());
    }
    std::vector<llvm::Value*> args_v;
    if(is_meth_call) {
        args_v.push_back(this_inst);
    }
    for(uint32_t i = 0; i != node.args.size(); ++i) {
        node.args[i]->accept(*this); auto arg = _ctx.ret_value;
        args_v.push_back(arg);
        if(!args_v.back()) {
            throw std::runtime_error("Could not emit function call argument");
        }
    }
    if(func_t->getReturnType() == llvm::Type::getVoidTy(_context)) {
        _builder.CreateCall(lhs, args_v);
        _ctx.ret_value = nullptr;
    } else {
        _ctx.ret_value = _builder.CreateCall(lhs, args_v, "__calltmp");
    }
}

Visitor::LLVM::Compiler::call_info Visitor::LLVM::Compiler::compile_call_lhs(const Parser::Nodes::Expression &lhs) {
    auto old_action = _ctx.ptr_action;
    auto old_context = _ctx.call_ctx;
    // set context
    _ctx.ptr_action = PtrAction::Load;
    _ctx.call_ctx = {true, nullptr, nullptr};
    //std::cerr << "CE: Getting lhs\n";
    lhs.accept(*this); auto call_lhs = _ctx.ret_value;
    //std::cerr << "CE: Got lhs\n";
    auto str_instance = _ctx.call_ctx.this_instance;
    auto named_function = _ctx.call_ctx.func;

    _ctx.call_ctx = old_context;    
    _ctx.ptr_action = old_action;
    return std::make_tuple(call_lhs, str_instance, named_function);
}


void Visitor::LLVM::Compiler::visit(const Parser::Nodes::IndexExpr &node) {
    //std::cerr << "Indexing\n";
    auto old_action = _ctx.ptr_action;
    _ctx.ptr_action = PtrAction::Address;
    node.lhs->accept(*this); auto lhs = _ctx.ret_value;
    _ctx.ptr_action = PtrAction::Load;
    if(strip_ptr_type(lhs)->isPointerTy()) {
        // pointer to index type
        // we can implicit deref it
        lhs = perform_ptr_action(lhs, nullptr, "__impl_deref_index");
    }
    node.index_expr->accept(*this); auto index = _ctx.ret_value;
    std::vector<llvm::Value*> gep_indexes{
        llvm::ConstantInt::get(_context, llvm::APInt(32, 0)),
        index};
    if(!lhs->getType()->isPointerTy()) {
        auto anon = create_anon_var(
            *_builder.GetInsertBlock()->getParent(),
            "__anonym_value",
            lhs->getType());
        auto old_action = _ctx.ptr_action;
        _ctx.ptr_action = PtrAction::Store;
        perform_ptr_action(anon, lhs, "__anonym_store");
        lhs = anon;
        _ctx.ptr_action = old_action;
    }
    auto gep = _builder.CreateGEP(lhs, gep_indexes, "__gep_adr");
    _ctx.ptr_action = old_action; // Retrieve old action here so we know if we want to load or just ptr
    _ctx.ret_value = perform_ptr_action(gep, nullptr, "__gep_val");
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::AccessExpr &node) {
    //std::cerr << "AE: Access expr\n";
    // get address of lhs for gep
    auto old_action = _ctx.ptr_action;
    _ctx.ptr_action = PtrAction::Address;
    //std::cerr << "AE: Getting lhs\n";
    node.lhs->accept(*this); auto lhs = _ctx.ret_value;
    //std::cerr << "AE: Got lhs\n";
    Parser::Nodes::Identifier* ident = nullptr;
    if(ident = dynamic_cast<Parser::Nodes::Identifier*>(node.rhs.get()); ident == nullptr) {
        throw std::runtime_error("Expected identifier for the access operator");
    }
    //std::cerr << "AE: Getting struct field\n";
    auto [field, is_method] = access_struct_field(lhs, ident->symbol);
    //std::cerr << "AE: Got struct field\n";    
    if(is_method) {
        //std::cerr << "AE: Its a method!\n";
        _ctx.ptr_action = old_action;
        // returning function ptr, we dont load it
        _ctx.ret_value = field;
        if(_ctx.call_ctx.is_call) {
            //std::cerr << "AE: setting call context\n";
            auto meth_id = meth_identifier(get_struct_type_name(lhs), ident->symbol);
            _ctx.call_ctx.func = &(_ctx.functions[meth_id]);

            auto str_type = lhs->getType();
            str_type = llvm::dyn_cast<llvm::PointerType>(str_type);
            if(str_type->getPointerElementType()->isPointerTy()) {
                _ctx.ptr_action = PtrAction::Load;
                // implicit deref struct ptr
                lhs = perform_ptr_action(lhs);
            }
            _ctx.call_ctx.this_instance = lhs;
            //std::cerr << "AE: set call context\n";
        }
        //std::cerr << "AE: Return\n";
    } else {
        //std::cerr << "AE: Its not a method!\n";
        _ctx.ptr_action = old_action; 
        _ctx.ret_value = perform_ptr_action(field, nullptr, "__gep_val");
    }
}

std::pair<llvm::Value*, bool> Visitor::LLVM::Compiler::access_struct_field(llvm::Value* str, const std::string& field_name) {
    // alloca ptr strip
    auto [s, s_wrapper] = get_struct_value_with_info(str);
    std::int32_t gep_index = s_wrapper->member_index(field_name);
    if(gep_index < 0) {
        // possibly a method
        //std::cerr << "AF: Possibly a method\n";
        auto it = s_wrapper->methods.find(field_name);
        if(it == s_wrapper->methods.end()) {
            throw std::runtime_error("Uknown struct field " + field_name);
        }
        //std::cerr << "AF: Found it " << it->second->func->identifier->symbol << "\n";
        if(it->second->llvm_func == nullptr) {
            //std::cerr << "AF: Method llvm_func is a nullptr!\n";
        }
        return std::make_pair((llvm::Value*)it->second->llvm_func, true);
    }
    // its a field and we have it
    std::vector<llvm::Value*> gep_indexes{
        llvm::ConstantInt::get(_context, llvm::APInt(32, 0)),
        llvm::ConstantInt::get(_context, llvm::APInt(32, (uint64_t)gep_index))};
    if(s->getType()->isStructTy()) {
        // its a struct value (for example returned from a function)
        // as we can only do gep on pointers we add it to the stack
        // to have a pointer to it  
        auto anon = create_anon_var(
            *_builder.GetInsertBlock()->getParent(),
            "__anonym_value",
            s->getType());
        auto old_action = _ctx.ptr_action;
        _ctx.ptr_action = PtrAction::Store;
        perform_ptr_action(anon, s, "__anonym_store");
        s = anon;
        _ctx.ptr_action = old_action;
    }
    auto gep = _builder.CreateGEP(s, gep_indexes, "__gep_adr");
    //std::cerr << "Gep done, we are happu :)\n";
    return std::make_pair(gep, false);
}

std::pair<llvm::Value*, Visitor::LLVM::Compiler::StructWrapper*> Visitor::LLVM::Compiler::get_struct_value_with_info(llvm::Value* str) {
    auto expr_type = strip_ptr_type(str);
    if(!(expr_type->isStructTy())) {
        // could be pointer to struct
        expr_type = strip_ptr_type(expr_type);
        if(!(expr_type->isStructTy())) {
            throw std::runtime_error("Access can only be done on structs");            
        }
        // implicit pointer dereference
        auto old_action = _ctx.ptr_action;
        _ctx.ptr_action = PtrAction::Load;
        str = perform_ptr_action(str, nullptr, "__impl_deref_struct_load");
        _ctx.ptr_action = old_action;
    }
    auto& s_wrapper = _ctx.structs[llvm::dyn_cast<llvm::StructType>(expr_type)->getName()];
    return std::make_pair(str, &s_wrapper);
}

std::string Visitor::LLVM::Compiler::get_struct_type_name(llvm::Value* str) {
    auto expr_type = strip_ptr_type(str);
    if(!(expr_type->isStructTy())) {
        // possible pointer to struct
        expr_type = strip_ptr_type(expr_type);
        if(!(expr_type->isStructTy())) {
            throw std::runtime_error("Not a struct type!");            
        }
    }
    return expr_type->getStructName();
}


// Primary
void Visitor::LLVM::Compiler::visit(const Parser::Nodes::Identifier &node) {
    if(auto var = get_local_var(node.symbol); var) {
        _ctx.ret_value = perform_ptr_action(var.value()->llvm_alloca, nullptr, node.symbol);
        return;
    }
    if(auto var = get_global_var(node.symbol); var) {
        _ctx.ret_value = perform_ptr_action(var.value()->llvm_var, nullptr, node.symbol);
        return;
    }
    if(auto func = get_function(node.symbol); func) {
        if(_ctx.call_ctx.is_call) {
            _ctx.call_ctx = {true, func.value(), nullptr};
        }
        _ctx.ret_value = func.value()->llvm_func;
        return;
    }
    if(auto var = get_struct_var(node.symbol); var) {
        _ctx.ret_value = perform_ptr_action(var.value(), nullptr, node.symbol);
        return;
    }
    if(auto meth = get_struct_method(node.symbol); meth) {
        if(_ctx.call_ctx.is_call) {
            _ctx.call_ctx.func = meth.value();
            auto old_action = _ctx.ptr_action;
            _ctx.ptr_action = PtrAction::Load;
            // this is a ptr to ptr in llvm so we load it once
            _ctx.call_ctx.this_instance = perform_ptr_action(
                _ctx.local_variables[_this_identifier].llvm_alloca);
            _ctx.ptr_action = old_action;
        }
        _ctx.ret_value = meth.value()->llvm_func;
        return;
    }
    throw std::runtime_error("Use of unknown identifier " + node.symbol);
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::ParenthesisExpr &node) {
    node.expr->accept(*this);
}

std::optional<Visitor::LLVM::Compiler::VarWrapper*> Visitor::LLVM::Compiler::get_local_var(const std::string& name) {
    if(auto it = _ctx.local_variables.find(name); it != _ctx.local_variables.end()) {
        return &it->second;
    }
    return std::nullopt;
}

std::optional<Visitor::LLVM::Compiler::FuncProtWrapper*> Visitor::LLVM::Compiler::get_function(const std::string& name) {
    if(auto it = _ctx.functions.find(name); it != _ctx.functions.end()) {
        return &it->second;
    }
    return std::nullopt;
}

std::optional<llvm::Value*> Visitor::LLVM::Compiler::get_struct_var(const std::string& name) {
    if(_ctx.curr_struct) {
        auto str_this = _ctx.local_variables[_this_identifier];
        auto [v, is_meth] = access_struct_field(str_this.llvm_alloca, name);
        if(!is_meth) {
            return v;
        }
    }
    return std::nullopt;
}

std::optional<Visitor::LLVM::Compiler::FuncProtWrapper*> Visitor::LLVM::Compiler::get_struct_method(const std::string& name) {
    if(_ctx.curr_struct) {
        auto str_this = _ctx.local_variables[_this_identifier];
        auto [v, is_meth] = access_struct_field(str_this.llvm_alloca, name);
        if(is_meth) {
            return &_ctx.functions[meth_identifier(name)];
        }
    }
    return std::nullopt;
} 

std::optional<Visitor::LLVM::Compiler::GlobalVarWrapper*> Visitor::LLVM::Compiler::get_global_var(const std::string& name) {
    if(auto it = _ctx.global_variables.find(name); it != _ctx.global_variables.end()) {
        return &it->second;
    }
    return std::nullopt;
}

// Consts
void Visitor::LLVM::Compiler::visit(const Parser::Nodes::IntConstant &node) {
    // todo for now ints are just 32 bits
    _ctx.ret_value = llvm::ConstantInt::get(_context, llvm::APInt(32, uint32_t(node.value)));
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::StringConstant &node) {
    throw std::runtime_error("No string support yet");
}

void Visitor::LLVM::Compiler::visit(const Parser::Nodes::FloatConstant &node) {
    // todo for now just doubles
    _ctx.ret_value = llvm::ConstantFP::get(_context, llvm::APFloat(node.value));
}


Visitor::LLVM::Compiler::VarWrapper& Visitor::LLVM::Compiler::create_local_var(
        llvm::Function &func,
        const Parser::Nodes::VariableDecl &node) {

    if(_ctx.local_variables.count(node.identifier->symbol) > 0) {
        throw std::runtime_error("Variable redeclaration " + node.identifier->symbol);
    }
    llvm::IRBuilder<> tmp_b(&func.getEntryBlock(), func.getEntryBlock().begin());
    auto alloca = tmp_b.CreateAlloca(
        Type::to_llvm(*node.type, _context, _ctx.structs),
        nullptr,
        node.identifier->symbol);
    _ctx.local_variables[node.identifier->symbol] = VarWrapper{&node, alloca};
    return _ctx.local_variables[node.identifier->symbol];

}

Visitor::LLVM::Compiler::VarWrapper& Visitor::LLVM::Compiler::create_local_var(
            llvm::Function &func,
            const std::string& identifier,
            llvm::Type* type) {

    if(_ctx.local_variables.count(identifier) > 0) {
        throw std::runtime_error("Variable redeclaration " + identifier);
    }
    llvm::IRBuilder<> tmp_b(&func.getEntryBlock(), func.getEntryBlock().begin());
    auto alloca = tmp_b.CreateAlloca(
        type,
        nullptr,
        identifier);
    _ctx.local_variables[identifier] = VarWrapper{nullptr, alloca};
    return _ctx.local_variables[identifier];
}

llvm::AllocaInst* Visitor::LLVM::Compiler::create_anon_var(
            llvm::Function &func,
            const std::string& identifier,
            llvm::Type* type) {
    llvm::IRBuilder<> tmp_b(&func.getEntryBlock(), func.getEntryBlock().begin());
    return tmp_b.CreateAlloca(
        type,
        nullptr,
        identifier);
}


llvm::Value *Visitor::LLVM::Compiler::cast(llvm::Value *from, llvm::Value *to) {
    //std::cerr << "Cast: " << from->getName().str() << " to: " << to->getName().str() << "\n";
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
    //std::cerr << "Cast ended!\n";
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

    switch (_ctx.ptr_action) {
        case PtrAction::None:
            //std::cerr << "Ptr action is None\n";
            return nullptr;
        case PtrAction::Store:
            //std::cerr << "Ptr action is store to " << ptr->getName().str() << "\n";
            if(!v) {
                throw std::runtime_error("Store of a null value");
            }
            return _builder.CreateStore(v, ptr);
        case PtrAction::Load:
            //std::cerr << "ptr action is load as " << load_s << " from: " << ptr->getName().str() <<"\n";
            return _builder.CreateLoad(ptr, load_s);
        case PtrAction::Address:
            //std::cerr << "ptr actions is address of " << ptr->getName().str() << "\n";
            return ptr;
    }
}


std::int32_t Visitor::LLVM::Compiler::StructWrapper::member_index(const std::string &name) const {
    for(std::uint32_t i = 0; i < str->members.size(); ++i) {
        if(name == str->members[i]->identifier->symbol) {
            return i;
        }
    }
    return -1;
}


