//
// Created by igor on 17.02.19.
//

#include <visitor/llvm.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>

#include <parser/nodes/concrete.h>

#include <iostream>
#include <cstdio>
#include <cstdlib>

#include <llvm/IR/Verifier.h>
#include <llvm/ADT/STLExtras.h>


// todo allocas only in the first block


// Base
void Visitor::LLVM::visit(const Parser::Nodes::Base &node) {
    throw std::runtime_error("Cannot compile base node!");
}


// End
// Program
void Visitor::LLVM::visit(const Parser::Nodes::Program &node) {
    // todo maybe init module here ?
    //std::cout << "Program node\n";
    //std::cout << "Program compilation started\n";

    node.accept_children(*this);

    std::cout << "Printing module\n\n\n";
    _module->print(llvm::outs(), nullptr);
}


// Top Level
// Function
void Visitor::LLVM::visit(const Parser::Nodes::FunctionProt &node) {
    //std::cout << "FunctionProt\n";
    if(_basic_types.count(node.type->identifier().symbol) == 0) {
        throw std::runtime_error("Usage of unsupported return type " + node.type->identifier().symbol);
    }
    //std::cout << "Compiling arg list\n";
    //std::cout << "Arg list size is " << node.arg_list.size() << "\n";
    std::vector<llvm::Type*> args_types;
    for(const auto& arg: node.arg_list) {
        if(_basic_types.count(arg->type->identifier().symbol) == 0) {
            throw std::runtime_error("Usage of undeclared type " + arg->type->identifier().symbol);
        }
        // add int64 type. todo need more types
        auto int_info = _int_types[arg->type->identifier().symbol];
        args_types.push_back(llvm::Type::getIntNTy(_context, int_info.size));
    }
    // return type is int. todo Later needs to use more than one type
    //std::cout << "Func arg types size: " << args_types.size() << "\n";
    //std::cout << "Compiling func type\n";
    auto int_info = _int_types[node.type->identifier().symbol];
    llvm::FunctionType* func_t = llvm::FunctionType::get(
            llvm::Type::getIntNTy(_context, int_info.size),
            args_types,
            false);
    //std::cout << "Compiling func header\n";
    llvm::Function* func = llvm::Function::Create(
            func_t,
            llvm::Function::ExternalLinkage,
            node.identifier->symbol,
            _module.get());

    if(!func) {
        throw std::runtime_error("Could not compile function!");
    }

    // todo its ugly, lets make it enumerate
    //std::cout << "Setting func arg names\n";
    unsigned int i = 0;
    for(auto& arg: func->args()) {
        //std::cout << "Setting name of arg: " << i << " " << node.arg_list[i]->identifier->symbol << "\n";
        arg.setName(node.arg_list[i++]->identifier->symbol);
    }
    //std::cout << "Printing func IR\n";
    // todo its just temporary
    _ret_func = func;
    func->print(llvm::outs());
    //std::cout << "\n";
}

void Visitor::LLVM::visit(const Parser::Nodes::FunctionDef &node) {
    //std::cout << "FuncDef\n";
    // todo there is a bug with redefining function with different arg names

    auto func = _module->getFunction(node.declaration->identifier->symbol);
    if(!func) {
        node.declaration->accept(*this); func = _ret_func;
    }
    if(!func) {
        throw std::runtime_error("Could not compile func prototype! " + node.declaration->identifier->symbol);
    }
    if(!func->empty()) {
        throw std::runtime_error("Redefinition of func " + node.declaration->identifier->symbol);
    }
    auto basic_block = llvm::BasicBlock::Create(_context, "entry", func);
    _builder.SetInsertPoint(basic_block);

    // todo why do we clear it?
    _named_values.clear();
    for(auto& arg: func->args()) {
        _named_values[arg.getName()] = _builder.CreateAlloca(arg.getType(), nullptr, arg.getName());

    }

    // visit the body
    // todo actually should subclass codeblock to function body
    // todo no func body parsing for now
    node.body->accept(*this);


    if(_ret_value) {
        //_builder.CreateRet(_ret_value);
        // no return here
        llvm::verifyFunction(*func);
        _ret_func = func;
    } else {
        // error on body, remove function
        func->eraseFromParent();
        _ret_func = nullptr;
    }
    func->print(llvm::outs());
}

void Visitor::LLVM::visit(const Parser::Nodes::ReturnStmt &node) {
    _ret_value = nullptr;
    node.expr->accept(*this);
    if(_ret_value) {
        _builder.CreateRet(_ret_value);
    }
}

// Statement
void Visitor::LLVM::visit(const Parser::Nodes::CodeBlock &node) {
    _ret_value = nullptr;
    for(auto& ch: node.children()) {
        ch->accept(*this);
    }
}


void Visitor::LLVM::visit(const Parser::Nodes::BlockStmt &node) {
    node.body->accept(*this);
}

// todo this should work?
void Visitor::LLVM::visit(const Parser::Nodes::VariableDecl &node) {
    ////std::cout << "Var decl\n";
    auto v = _named_values[node.identifier->symbol];
    if(v) {
        throw std::runtime_error("Redeclaration of a variable! " + node.identifier->symbol);
    }
    if(_basic_types.count(node.type->identifier().symbol) == 0) {
        throw std::runtime_error("Use of undeclared type " + node.type->identifier().symbol);
    }
    // todo for now only int32
    // todo later to_llvm_type function
    _named_values[node.identifier->symbol] = _builder.CreateAlloca(
        llvm::Type::getInt32Ty(_context),
        nullptr,
        node.identifier->symbol);
    if(node.init_expr) {
        node.init_expr->accept(*this); auto init = _ret_value;
        if(!init) {
            throw std::runtime_error("Could not compile variable init expr");
        }
        _builder.CreateStore(init, _named_values[node.identifier->symbol]);
    }
}

void Visitor::LLVM::visit(const Parser::Nodes::IfStmt &node) {
    node.cond->accept(*this); auto cond = _ret_value;
    auto func = _builder.GetInsertBlock()->getParent();
    auto then = llvm::BasicBlock::Create(_context, "then", func);

    llvm::BasicBlock* else_clause = nullptr;
    if (node.else_clause) {
        else_clause = llvm::BasicBlock::Create(_context, "else", func);
    }
    auto merge = llvm::BasicBlock::Create(_context, "merge", func);

    if(else_clause) {
        _builder.CreateCondBr(cond, then, else_clause);
    } else {
        _builder.CreateCondBr(cond, then, merge);
    }

    _builder.SetInsertPoint(then);
    node.body->accept(*this);
    _builder.CreateBr(merge);
    then = _builder.GetInsertBlock();

    if(node.else_clause) {
        _builder.SetInsertPoint(else_clause);
        node.else_clause->accept(*this);
        _builder.CreateBr(merge);
        else_clause = _builder.GetInsertBlock();
    }

    _builder.SetInsertPoint(merge);
}


// Expression
// Binary
void Visitor::LLVM::visit(const Parser::Nodes::LogicalOrExpr &node) {
    if(node.op.id != Lexer::Token::Id::None) {
        throw std::runtime_error("Unsupported binary operator");
    }
    node.lhs->accept(*this);
}

void Visitor::LLVM::visit(const Parser::Nodes::LogicalAndExpr &node) {
    if(node.op.id != Lexer::Token::Id::None) {
        throw std::runtime_error("Unsupported binary operator");
    }
    node.lhs->accept(*this);
}

void Visitor::LLVM::visit(const Parser::Nodes::InclusiveOrExpr &node) {
    if(node.op.id != Lexer::Token::Id::None) {
        throw std::runtime_error("Unsupported binary operator");
    }
    node.lhs->accept(*this);
}

void Visitor::LLVM::visit(const Parser::Nodes::ExclusiveOrExpr &node) {
    if(node.op.id != Lexer::Token::Id::None) {
        throw std::runtime_error("Unsupported binary operator");
    }
    node.lhs->accept(*this);
}

void Visitor::LLVM::visit(const Parser::Nodes::AndExpr &node) {
    if(node.op.id != Lexer::Token::Id::None) {
        throw std::runtime_error("Unsupported binary operator");
    }
    node.lhs->accept(*this);
}

void Visitor::LLVM::visit(const Parser::Nodes::EqualityExpr &node) {
    if(node.op.id != Lexer::Token::Id::None) {
        throw std::runtime_error("Unsupported binary operator");
    }
    node.lhs->accept(*this);
}


// todo for now only signed cmp
// todo we dont now how to make unsigned ints
// todo in future versions we need to know the type of the lhs
// todo and the rhs to make appropriate cmp
void Visitor::LLVM::visit(const Parser::Nodes::RelationalExpr &node) {
    //std::cout << "Relational expr\n";
    node.lhs->accept(*this); auto lhs = _ret_value;
    llvm::Value* rhs = nullptr;
    if(node.rhs) {
        node.rhs->accept(*this); rhs = _ret_value;
    }
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
        case Lexer::Token::Id::None:
            _ret_value = lhs;
            break;
        default:
            throw std::runtime_error("Unexpected operator during addition operator");
    }
}

void Visitor::LLVM::visit(const Parser::Nodes::ShiftExpr &node) {
    if(node.op.id != Lexer::Token::Id::None) {
        throw std::runtime_error("Unsupported binary operator");
    }
    node.lhs->accept(*this);
}

void Visitor::LLVM::visit(const Parser::Nodes::AssignmentExpr &node) {
    //std::cout << "Assig expr\n";
    // todo for now only identifier
    // todo support for access and indexing operators

    if (node.rhs) {
        // its assignment
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
        _builder.CreateStore(rhs, lhs);
        // leaving lhs in _ret_value, so assignment evaluates to it
    } else {
        // its just some random expr
        // just pass it
        node.lhs->accept(*this);
    }
}

void Visitor::LLVM::visit(const Parser::Nodes::AdditiveExpr &node) {
    // Compute left and right if needed
    //std::cout << "Add expr\n";
    node.lhs->accept(*this); auto lhs = _ret_value;
    llvm::Value* rhs = nullptr;
    if(node.rhs) {
        node.rhs->accept(*this); rhs = _ret_value;
    }
    switch(node.op.id) {
        case Lexer::Token::Id::Plus:
            _ret_value = _builder.CreateAdd(lhs, rhs, "__addtmp");
            break;
        case Lexer::Token::Id::Minus:
            _ret_value = _builder.CreateSub(lhs, rhs, "__addtmp");
            break;
        case Lexer::Token::Id::None:
            _ret_value = lhs;
            break;
        default:
            throw std::runtime_error("Unexpected operator during addition operator");
    }
}

void Visitor::LLVM::visit(const Parser::Nodes::MultiplicativeExpr &node) {
    //std::cout << "Mult expr\n";
    node.lhs->accept(*this); auto lhs = _ret_value;
    llvm::Value* rhs = nullptr;
    if(node.rhs) {
        node.rhs->accept(*this); rhs = _ret_value;
    }
    switch(node.op.id) {
        case Lexer::Token::Id::Multiplication:
            _ret_value = _builder.CreateMul(lhs, rhs, "__multmp");
            break;
        case Lexer::Token::Id::Division:
            // todo for now its unsigned division, we can do signed division
            // but its kinda more comlpicated
            _ret_value = _builder.CreateUDiv(lhs, rhs, "__multmp");
            break;
        case Lexer::Token::Id::None:
            _ret_value = lhs;
            break;
        default:
            throw std::runtime_error("Unexpected operator during addition operator");
    }
}


// Unary
void Visitor::LLVM::visit(const Parser::Nodes::UnaryExpr &node) {
    //std::cout << "Unary\n";
    if(node.op.id != Lexer::Token::Id::None) {
        throw std::runtime_error("Unsupported unary operator");
    }
    node.rhs->accept(*this);
}


// Postfix
void Visitor::LLVM::visit(const Parser::Nodes::PostfixExpr &node) {
    //std::cout << "Postifx\n";
    node.lhs->accept(*this);
}

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

    _ret_value = _builder.CreateCall(callee, args_v, "__calltmp");
}

// Primary
void Visitor::LLVM::visit(const Parser::Nodes::Identifier &node) {
    //std::cout << "IdentifierExpr\n";
    llvm::Value *v = _named_values[node.symbol];
    if(!v) {
        // possible function call, we pass the identifier
        // todo make it work somehow later
        _ret_symbol = node.symbol;
        _ret_value = nullptr;
        return;
    }
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
    _ret_value = llvm::ConstantInt::get(_context, llvm::APInt(32, uint32_t(node.value)));
}








