//
// Created by igor on 30.12.18.
//

#include <iostream>

#include <visitor/log.h>
#include <lexer/lexer.h>


Visitor::Stringify::Stringify(Parser::AST &ast) {
    _ast = std::move(std::make_optional(std::ref(ast)));
    add_header();
}

void Visitor::Stringify::add_header() {
    _stream << "AST information\n";
    _stream << "Structs\n";
    for(auto& s: _ast->get().iter_struct_decl()) {
        _stream << "\t\t" << s.first << "\n";
    }
    _stream << "GlobVars\n";
    for(auto& s: _ast->get().iter_glob_var_decl()) {
        _stream << "\t\t" << s.first << "\n";
    }
    _stream << "FuncProtos\n";
    for(auto& s: _ast->get().iter_func_prot()) {
        _stream << "\t\t" << s.first << "\n";
    }
    _stream << "FuncDefs\n";
    for(auto& s: _ast->get().iter_func_def()) {
        _stream << "\t\t" << s.first << "\n";
    }
}

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
    stringify(node, "GlobVarDecl: " + strf_type(node.type) + " " + node.identifier->symbol);
}

void Visitor::Stringify::visit(const Parser::Nodes::StructDecl &node) {
    std::string mess = "StructDecl: " + node.identifier->symbol;
    if(!node.wrapped_structs.empty()) {
        mess += " wraps ";
        for(auto& s: node.wrapped_structs) {
            mess += s->symbol + ", ";
        }
    }
    stringify(node, std::move(mess));

    for(auto& m: node.members) {
        m->accept(*this);
    }
    for(auto& m: node.methods) {
        m->accept(*this);
    }
}


// Function
void Visitor::Stringify::visit(const Parser::Nodes::FunctionProt &node) {
    std::string repr{"FuncHeader: " + strf_type(node.type) + " " + node.identifier->symbol + "("};
    for(const auto& arg: node.arg_list) {
        repr += strf_type(arg->type) + " " + arg->identifier->symbol + ", ";
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
    stringify(node, "VarDecl: " + strf_type(node.type) + " " + node.identifier->symbol);
    if(node.init_expr) {
        node.init_expr->accept(*this);
    }
}


void Visitor::Stringify::visit(const Parser::Nodes::BlockStmt &node) {
    stringify(node, "BlockStmt");
    node.body->accept(*this);
}

void Visitor::Stringify::visit(const Parser::Nodes::IfStmt &node) {
    stringify(node, "IfStmt");
    node.cond->accept(*this);
    node.body->accept(*this);
    if(node.else_clause) {
        node.else_clause->accept(*this);
    }
}

void Visitor::Stringify::visit(const Parser::Nodes::WhileStmt &node) {
    stringify(node, "WhileStmt");
    node.cond->accept(*this);
    node.body->accept(*this);
}

void Visitor::Stringify::visit(const Parser::Nodes::ReturnStmt &node) {
    stringify(node, "ReturnStmt");
    if(node.expr) {
        node.expr->accept(*this);
    }
}

void Visitor::Stringify::visit(const Parser::Nodes::BreakStmt &node) {
    stringify(node, "BreakStmt");
}

void Visitor::Stringify::visit(const Parser::Nodes::ContinueStmt &node) {
    stringify(node, "ContinueStmt");
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

// Logical
void Visitor::Stringify::visit(const Parser::Nodes::LogicalOrExpr &node) {
    std::string mess{"LogicalOrExpr: "};
    if(node.op.id == Lexer::Token::Id::LogicalOr) {
        mess += "||";
    } else {
        mess += "None";
    }
    stringify(node, std::move(mess));
    node.lhs->accept(*this);
    if(node.rhs) {
        node.rhs->accept(*this);
    }
}

void Visitor::Stringify::visit(const Parser::Nodes::LogicalAndExpr &node) {
    std::string mess{"LogicalAndExpr: "};
    if(node.op.id == Lexer::Token::Id::LogicalAnd) {
        mess += "&&";
    } else {
        mess += "None";
    }
    stringify(node, std::move(mess));
    node.lhs->accept(*this);
    if(node.rhs) {
        node.rhs->accept(*this);
    }
}

void Visitor::Stringify::visit(const Parser::Nodes::InclusiveOrExpr &node) {
    std::string mess{"InclusiveOrExpr: "};
    if(node.op.id == Lexer::Token::Id::InclusiveOr) {
        mess += "|";
    } else {
        mess += "None";
    }
    stringify(node, std::move(mess));
    node.lhs->accept(*this);
    if(node.rhs) {
        node.rhs->accept(*this);
    }
}

void Visitor::Stringify::visit(const Parser::Nodes::ExclusiveOrExpr &node) {
    std::string mess{"ExclusiveOrExpr: "};
    if(node.op.id == Lexer::Token::Id::ExclusiveOr) {
        mess += "^";
    } else {
        mess += "None";
    }
    stringify(node, std::move(mess));
    node.lhs->accept(*this);
    if(node.rhs) {
        node.rhs->accept(*this);
    }
}

void Visitor::Stringify::visit(const Parser::Nodes::AndExpr &node) {
    std::string mess{"AndExpr: "};
    if(node.op.id == Lexer::Token::Id::And) {
        mess += "&";
    } else {
        mess += "None";
    }
    stringify(node, std::move(mess));
    node.lhs->accept(*this);
    if(node.rhs) {
        node.rhs->accept(*this);
    }
}

void Visitor::Stringify::visit(const Parser::Nodes::EqualityExpr &node) {
    std::string mess = "EqualityExpr: ";
    if(node.op.id == Lexer::Token::Id::Equality) {
        mess += "==";
    } else if(node.op.id == Lexer::Token::Id::Inequality) {
        mess += "!=";
    } else {
        mess += "None";
    }
    stringify(node, std::move(mess));
    node.lhs->accept(*this);
    if(node.rhs) {
        node.rhs->accept(*this);
    }
}

void Visitor::Stringify::visit(const Parser::Nodes::RelationalExpr &node) {
    std::string mess = "RelationalExpr: ";
    if(node.op.id == Lexer::Token::Id::GreaterThan) {
        mess += ">";
    } else if(node.op.id == Lexer::Token::Id::GreaterEq) {
        mess += ">=";
    } else if(node.op.id == Lexer::Token::Id::LessThan) {
        mess += "<";
    } else if(node.op.id == Lexer::Token::Id::LessEq) {
        mess += "<=";
    } else {
        mess += "None";
    }
    stringify(node, std::move(mess));
    node.lhs->accept(*this);
    if(node.rhs) {
        node.rhs->accept(*this);
    }
}

void Visitor::Stringify::visit(const Parser::Nodes::ShiftExpr &node) {
    std::string mess = "ShiftExpr: ";
    if(node.op.id == Lexer::Token::Id::LeftShift) {
        mess += "<<";
    } else if(node.op.id == Lexer::Token::Id::RightShift) {
        mess += ">>";
    } else {
        mess += "None";
    }
    stringify(node, std::move(mess));
    node.lhs->accept(*this);
    if(node.rhs) {
        node.rhs->accept(*this);
    }
}


// Arithmetic
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
void Visitor::Stringify::visit(const Parser::Nodes::UnaryExpr &node) {
    stringify(node, "UnaryExpr");
    node.rhs->accept(*this);
}

void Visitor::Stringify::visit(const Parser::Nodes::NegativeExpr &node) {
    stringify(node, "Negative: -" );
    node.rhs->accept(*this);
}

void Visitor::Stringify::visit(const Parser::Nodes::NegationExpr &node) {
    stringify(node, "Negation: !");
    node.rhs->accept(*this);
}

void Visitor::Stringify::visit(const Parser::Nodes::AddressAccessExpr &node) {
    stringify(node, "AdressAccess: &");
    node.rhs->accept(*this);
}
void Visitor::Stringify::visit(const Parser::Nodes::DereferenceExpr &node) {
    stringify(node, "Dereference: val");
    node.rhs->accept(*this);
}

// Postfix
void Visitor::Stringify::visit(const Parser::Nodes::PostfixExpr &node) {
    stringify(node, "PostfixExpr");
    node.lhs->accept(*this);
}


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
    node.index_expr->accept(*this);
}

void Visitor::Stringify::visit(const Parser::Nodes::AccessExpr &node) {
    stringify(node, "AccessExpr: .");
    node.lhs->accept(*this);
    node.rhs->accept(*this);
}

void Visitor::Stringify::visit(const Parser::Nodes::CastExpr &node) {
    std::string mess = "CastExpr: " + strf_type(node.type);
    stringify(node, std::move(mess));
    node.lhs->accept(*this);
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

void Visitor::Stringify::visit(const Parser::Nodes::FloatConstant &node) {
    std::string mess = "FloatConst: " + std::to_string(node.value);
    stringify(node, std::move(mess));
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

std::string Visitor::Stringify::strf_type(const std::unique_ptr<Parser::Types::BaseType> &type) {
    if(auto complex = dynamic_cast<Parser::Types::ComplexType*>(type.get()); complex) {
        std::string mess = complex->is_const ? "const ": "";
        mess += complex->is_ptr ? "&" : "";
        mess += strf_type(complex->underlying_type);
        return mess;
    }
    if(auto simple = dynamic_cast<Parser::Types::SimpleType*>(type.get()); simple) {
        return simple->identifier().symbol;
    }
    if(auto array = dynamic_cast<Parser::Types::ArrayType*>(type.get()); array) {
        std::string mess = "[" + std::to_string(array->size) + "]";
        return mess + strf_type(array->underlying_type);
    }
    throw std::runtime_error("StringifyVisitor: Passed type is neither complex nor simple");
}


