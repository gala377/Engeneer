//
// Created by igor on 30.12.18.
//

#include <iostream>

#include <visitor/log.h>
#include <lexer/lexer.h>


// Visitor Interface

// Base
void Visitor::Stringify::visit(const Parser::Nodes::Base& node) {
    stringify(node, "Unknown node");
}


// End


// Program
void Visitor::Stringify::visit(const Parser::Nodes::Program& node) {
    stringify(node, "Program");
    node.accept_children(*this);
}


// Top Level
void Visitor::Stringify::visit(const Parser::Nodes::GlobVariableDecl &node) {
    stringify(node, "GlobVarDecl: " + node.type_identifier + " " + node.identifier);
}


// Function
void Visitor::Stringify::visit(const Parser::Nodes::FunctionProt &node) {
    std::string repr{"FuncHeader: " + node.type_identifier + " " + node.identifier + "("};
    for(const auto& arg: node.arg_list) {
        repr += arg->type_identifier + " " + arg->identifier + ", ";
    }
    repr += ")";
    stringify(node, std::move(repr));
}

void Visitor::Stringify::visit(const Parser::Nodes::FunctionDecl& node) {
    stringify(node, "FuncDecl");
}

void Visitor::Stringify::visit(const Parser::Nodes::FunctionDef &node) {
    stringify(node, "FuncDef");
    node.declaration->accept(*this);
    node.body->accept(*this);
}


// Statement
void Visitor::Stringify::visit(const Parser::Nodes::CodeBlock &node) {
    stringify(node, "CodeBlock");
    node.accept_children(*this);
}

void Visitor::Stringify::visit(const Parser::Nodes::VariableDecl &node) {
    stringify(node, "VarDecl: " + node.type_identifier + " " + node.identifier);
}


// Expression


// Binary
void Visitor::Stringify::visit(const Parser::Nodes::BinaryExpr &node) {
    stringify(node, "BinExpr");
    node.lhs->accept(*this);
    if(node.rhs) {
        node.rhs->accept(*this);
    }
}

void Visitor::Stringify::visit(const Parser::Nodes::AssignmentExpr &node) {
    std::string mess = "AssignmentExpr: ";
    if(node.op.id == Lexer::Token::Id::Assignment) {
        mess += "=";
    } else {
        mess += "None";
    }
    stringify(node, std::move(mess));
    node.lhs->accept(*this);
    if(node.rhs) {
        node.rhs->accept(*this);
    }
}

void Visitor::Stringify::visit(const Parser::Nodes::AdditiveExpr &node) {
    std::string mess = "AdditiveExpr: ";
    if(node.op.id == Lexer::Token::Id::Plus) {
        mess += "+";
    } else if(node.op.id == Lexer::Token::Id::Minus) {
        mess += "-";
    } else {
        mess += "None";
    }
    stringify(node, std::move(mess));
    node.lhs->accept(*this);
    if(node.rhs) {
        node.rhs->accept(*this);
    }
}

void Visitor::Stringify::visit(const Parser::Nodes::MultiplicativeExpr &node) {
    std::string mess = "MultiplicativeExpr: ";
    if(node.op.id == Lexer::Token::Id::Multiplication) {
        mess += "*";
    } else if(node.op.id == Lexer::Token::Id::Division) {
        mess += "/";
    } else {
        mess += "None";
    }
    stringify(node, std::move(mess));
    node.lhs->accept(*this);
    if(node.rhs) {
        node.rhs->accept(*this);
    }
}


// Unary
void Visitor::Stringify::visit(const Parser::Nodes::NegativeExpr &node) {
    stringify(node, "Negative: -" );
    node.rhs->accept(*this);
}


// Postfix
void Visitor::Stringify::visit(const Parser::Nodes::CallExpr &node) {
    stringify(node, "CallExpr: ()");
    node.lhs->accept(*this);
    for(auto& arg: node.args) {
        arg->accept(*this);
    }
}

void Visitor::Stringify::visit(const Parser::Nodes::IndexExpr &node) {
    stringify(node, "Index Expr: []");
    node.lhs->accept(*this);
}

void Visitor::Stringify::visit(const Parser::Nodes::AccessExpr &node) {
    stringify(node, "AccessExpr: .");
    node.lhs->accept(*this);
    node.rhs->accept(*this);
}


// Primary
void Visitor::Stringify::visit(const Parser::Nodes::Identifier &node) {
    stringify(node, "Identifier: " + node.symbol);
}

void Visitor::Stringify::visit(const Parser::Nodes::ParenthesisExpr &node) {
    stringify(node, "Parenthesis: ()");
    node.expr->accept(*this);
}


// Const
void Visitor::Stringify::visit(const Parser::Nodes::IntConstant &node) {
    std::string mess = "IntConst: " + std::to_string(node.value);
    stringify(node, std::move(mess));
}

void Visitor::Stringify::visit(const Parser::Nodes::StringConstant &node) {
    stringify(node, "StringConst: " + node.value);
}



// Class Interface
std::string Visitor::Stringify::repr() {
    return _stream.str();
}

void Visitor::Stringify::stringify(const Parser::Nodes::Base &node, std::string &&as) {
    add_margin(node.depth());
    _stream << as << "\n";
}

void Visitor::Stringify::add_margin(std::uint32_t depth) {
    for(std::uint32_t i = 0; i < depth; ++i) {
        _stream << "--------";
    }
}















