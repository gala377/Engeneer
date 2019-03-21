//
// Created by igor on 30.12.18.
//

#include <iostream>

#include <parser/nodes/concrete.h>


// End
void Parser::Nodes::End::accept(Parser::Visitor &v) const {
    v.visit(*this);
}



// Program
void Parser::Nodes::Program::accept(Parser::Visitor &v) const {
    v.visit(*this);
}



// Top Level
void Parser::Nodes::TopLevelDecl::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


// GlobVarDecl
Parser::Nodes::GlobVariableDecl::GlobVariableDecl(
        std::unique_ptr<Identifier>&& identifier,
        std::unique_ptr<Types::BaseType>&& type,
        std::unique_ptr<Expression>&& init_expr):
        identifier(std::move(identifier)),
        type(std::move(type)),
        init_expr(std::move(init_expr)) {}

void Parser::Nodes::GlobVariableDecl::accept(Parser::Visitor &v) const {
    v.visit(*this);
}

void Parser::Nodes::GlobVariableDecl::set_depth(std::uint32_t depth) {
    Base::set_depth(depth);
    if(init_expr) {
        init_expr->set_depth(_depth + 1);
    }
}


// Functions
void Parser::Nodes::FunctionDecl::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


Parser::Nodes::FunctionProt::FunctionProt(
        std::unique_ptr<Identifier>&& identifier,
        std::unique_ptr<Types::BaseType>&& type,
        std::vector<std::unique_ptr<Parser::Nodes::VariableDecl>> &&arg_list) :
        identifier(std::move(identifier)),
        type(std::move(type)),
        arg_list(std::move(arg_list)) {}

void Parser::Nodes::FunctionProt::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


Parser::Nodes::FunctionDef::FunctionDef(
        std::unique_ptr<Parser::Nodes::FunctionProt> &&decl,
        std::unique_ptr<Parser::Nodes::CodeBlock> &&body) :
        declaration(std::move(decl)), body(std::move(body)) {
    this->declaration->set_depth(_depth+1);
    this->body->set_depth(_depth+1);
}

void Parser::Nodes::FunctionDef::set_depth(std::uint32_t depth) {
    FunctionDecl::set_depth(depth);
    declaration->set_depth(_depth+1);
    body->set_depth(_depth+1);
}

void Parser::Nodes::FunctionDef::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


// Struct
Parser::Nodes::StructDecl::StructDecl(std::unique_ptr<Identifier>&& identifier,
                                      std::vector<std::unique_ptr<Parser::Nodes::VariableDecl>>&& members,
                                      std::vector<std::unique_ptr<Parser::Nodes::FunctionDecl>>&& methods,
                                      std::vector<std::unique_ptr<Identifier>>&& wrapped_structs):
                                      identifier{std::move(identifier)},
                                      members{std::move(members)},
                                      methods{std::move(methods)},
                                      wrapped_structs{std::move(wrapped_structs)} {}

void Parser::Nodes::StructDecl::accept(Parser::Visitor &v) const {
    v.visit(*this);
}

void Parser::Nodes::StructDecl::set_depth(std::uint32_t depth) {
    Base::set_depth(depth);
    for(auto& m: members) {
        m->set_depth(_depth+1);
    }
    for(auto& m: methods) {
        m->set_depth(_depth+1);
    }
}


// Statement
void Parser::Nodes::Statement::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


void Parser::Nodes::CodeBlock::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


Parser::Nodes::VariableDecl::VariableDecl(
        std::unique_ptr<Identifier>&& identifier,
        std::unique_ptr<Types::BaseType>&& type,
        std::unique_ptr<Parser::Nodes::Expression>&& init_expr):
        identifier(std::move(identifier)),
        type(std::move(type)),
        init_expr(std::move(init_expr)) {}

void Parser::Nodes::VariableDecl::accept(Parser::Visitor &v) const {
    v.visit(*this);
}

void Parser::Nodes::VariableDecl::set_depth(std::uint32_t depth) {
    Base::set_depth(depth);
    if(init_expr) {
        init_expr->set_depth(_depth + 1);
    }
}


Parser::Nodes::BlockStmt::BlockStmt(std::unique_ptr<Parser::Nodes::CodeBlock> &&body): body(std::move(body)) {}

void Parser::Nodes::BlockStmt::accept(Parser::Visitor &v) const {
    v.visit(*this);
}

void Parser::Nodes::BlockStmt::set_depth(std::uint32_t depth) {
    Base::set_depth(depth);
    body->set_depth(_depth + 1);
}


Parser::Nodes::IfStmt::IfStmt(std::unique_ptr<Parser::Nodes::Expression> &&cond,
                              std::unique_ptr<Parser::Nodes::CodeBlock> &&body,
                              std::unique_ptr<Parser::Nodes::BlockStmt> &&else_clause):
                              BlockStmt(std::move(body)),
                              cond(std::move(cond)),
                              else_clause(std::move(else_clause)) {}

void Parser::Nodes::IfStmt::set_depth(std::uint32_t depth) {
    BlockStmt::set_depth(depth);
    cond->set_depth(_depth+1);
    if(else_clause) {
        else_clause->set_depth(_depth+1);
    }
}

void Parser::Nodes::IfStmt::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


Parser::Nodes::WhileStmt::WhileStmt(std::unique_ptr<Parser::Nodes::Expression> &&cond,
                                    std::unique_ptr<Parser::Nodes::CodeBlock> &&body):
                                    BlockStmt(std::move(body)),
                                    cond(std::move(cond)) {}

void Parser::Nodes::WhileStmt::set_depth(std::uint32_t depth) {
    BlockStmt::set_depth(depth);
    cond->set_depth(_depth+1);
}

void Parser::Nodes::WhileStmt::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


Parser::Nodes::ReturnStmt::ReturnStmt(std::unique_ptr<Parser::Nodes::Expression> &&expr):
                                      expr(std::move(expr)) {}

void Parser::Nodes::ReturnStmt::accept(Parser::Visitor &v) const {
    v.visit(*this);
}

void Parser::Nodes::ReturnStmt::set_depth(std::uint32_t depth) {
    Base::set_depth(depth);
    if(expr) {
        expr->set_depth(_depth + 1);
    }
}


void Parser::Nodes::BreakStmt::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


void Parser::Nodes::ContinueStmt::accept(Parser::Visitor &v) const {
    v.visit(*this);
}



// Expressions
void Parser::Nodes::Expression::accept(Parser::Visitor &v) const {
    v.visit(*this);
}



// Binary
Parser::Nodes::BinaryExpr::BinaryExpr(
        std::unique_ptr<Parser::Nodes::Expression> &&lhs,
        const Lexer::Token& op,
        std::unique_ptr<Parser::Nodes::Expression> &&rhs):
        lhs(std::move(lhs)), rhs(std::move(rhs)), op(op) {}

void Parser::Nodes::BinaryExpr::set_depth(std::uint32_t depth) {
    Base::set_depth(depth);
    lhs->set_depth(_depth+1);
    if(rhs) {
        rhs->set_depth(_depth+1);
    }
}

void Parser::Nodes::BinaryExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


void Parser::Nodes::LogicalAndExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


void Parser::Nodes::LogicalOrExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


void Parser::Nodes::InclusiveOrExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


void Parser::Nodes::ExclusiveOrExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


void Parser::Nodes::AndExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


void Parser::Nodes::EqualityExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


void Parser::Nodes::RelationalExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


void Parser::Nodes::ShiftExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


void Parser::Nodes::AssignmentExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


void Parser::Nodes::AdditiveExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


void Parser::Nodes::MultiplicativeExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}



// Unary
Parser::Nodes::UnaryExpr::UnaryExpr(
        const Lexer::Token& op,
        std::unique_ptr<Parser::Nodes::Expression> &&rhs):
        op(op), rhs(std::move(rhs)) {}

void Parser::Nodes::UnaryExpr::set_depth(std::uint32_t depth) {
    Base::set_depth(depth);
    rhs->set_depth(_depth + 1);
}

void Parser::Nodes::UnaryExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


void Parser::Nodes::NegativeExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}

void Parser::Nodes::NegationExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}

void Parser::Nodes::AddressAccessExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}

// Postfix
Parser::Nodes::PostfixExpr::PostfixExpr(std::unique_ptr<Parser::Nodes::Expression> &&lhs):
    lhs(std::move(lhs)) {}

void Parser::Nodes::PostfixExpr::set_depth(std::uint32_t depth) {
    Base::set_depth(depth);
    lhs->set_depth(_depth+1);
}

void Parser::Nodes::PostfixExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


Parser::Nodes::CallExpr::CallExpr(std::unique_ptr<Parser::Nodes::Expression> &&lhs,
                                  std::vector<std::unique_ptr<Parser::Nodes::Expression>> &&args):
                                  PostfixExpr(std::move(lhs)), args(std::move(args)){}


void Parser::Nodes::CallExpr::set_depth(std::uint32_t depth) {
    PostfixExpr::set_depth(depth);
    for(auto& ch: args) {
        ch->set_depth(_depth+1);
    }
}

void Parser::Nodes::CallExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


Parser::Nodes::IndexExpr::IndexExpr(std::unique_ptr<Parser::Nodes::Expression> &&lhs,
                                    std::unique_ptr<Parser::Nodes::Expression> &&index_expr):
                                   PostfixExpr(std::move(lhs)), index_expr(std::move(index_expr)) {}

void Parser::Nodes::IndexExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}

void Parser::Nodes::IndexExpr::set_depth(std::uint32_t depth) {
    PostfixExpr::set_depth(depth);
    index_expr->set_depth(_depth+1);
}


Parser::Nodes::AccessExpr::AccessExpr(std::unique_ptr<Parser::Nodes::Expression> &&lhs,
                                      std::unique_ptr<Parser::Nodes::Expression> &&rhs):
                                      PostfixExpr(std::move(lhs)), rhs(std::move(rhs)) {}

void Parser::Nodes::AccessExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}

void Parser::Nodes::AccessExpr::set_depth(std::uint32_t depth) {
    PostfixExpr::set_depth(depth);
    rhs->set_depth(_depth+1);
}




// Primary
void Parser::Nodes::PrimaryExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


Parser::Nodes::Identifier::Identifier(const std::string& symbol) : symbol(symbol) {};

void Parser::Nodes::Identifier::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


Parser::Nodes::ParenthesisExpr::ParenthesisExpr(std::unique_ptr<Parser::Nodes::Expression> &&expr):
        expr(std::move(expr)) {
    this->expr->set_depth(_depth + 1);
}

void Parser::Nodes::ParenthesisExpr::set_depth(std::uint32_t depth) {
    Base::set_depth(depth);
    expr->set_depth(_depth+1);
}

void Parser::Nodes::ParenthesisExpr::accept(Parser::Visitor &v) const {
    v.visit(*this);
}



// Consts
void Parser::Nodes::Constant::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


Parser::Nodes::IntConstant::IntConstant(int value): value(value) {}

void Parser::Nodes::IntConstant::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


Parser::Nodes::StringConstant::StringConstant(const std::string& value): value(value) {}

void Parser::Nodes::StringConstant::accept(Parser::Visitor &v) const {
    v.visit(*this);
}


Parser::Nodes::FloatConstant::FloatConstant(double value): value(value) {}

void Parser::Nodes::FloatConstant::accept(Parser::Visitor &v) const {
    v.visit(*this);
}
