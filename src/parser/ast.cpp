//
// Created by igor on 30.12.18.
//

#include "parser/nodes/concrete.h"
#include <iostream>

#include <parser/ast.h>

void Parser::AST::accept(Parser::Visitor &v) {
    _root.accept(v);
}

Parser::Nodes::BaseParent& Parser::AST::root() {
    return _root; 
}


void Parser::AST::note(Parser::Nodes::FunctionProt *f) {
    if(auto res = function_protos.find(f->identifier->symbol); res != function_protos.end()) {
        throw std::runtime_error("Function redeclaration");
    }
    function_protos[f->identifier->symbol] = f;
}

void Parser::AST::note(Parser::Nodes::FunctionDef *f) {
    if(auto res = function_defs.find(f->declaration->identifier->symbol); res != function_defs.end()) {
        throw std::runtime_error("Function redeclaration");
    }
    function_defs[f->declaration->identifier->symbol] = f;
    // todo avoid collision with global variables
    note(f->declaration.get());
}

void Parser::AST::note(Parser::Nodes::GlobVariableDecl *v) {
    if(auto res = glob_var_decls.find(v->identifier->symbol); res != glob_var_decls.end()) {
        throw std::runtime_error("Variable redeclaration");
    }
    // todo avoid collisions with function names 
    glob_var_decls[v->identifier->symbol] = v;
}

void Parser::AST::note(Parser::Nodes::StructDecl *s) {
    if(auto res = structs_decls.find(s->identifier->symbol); res != structs_decls.end()) {
        throw std::runtime_error("Struct redeclaration");
    }
    structs_decls[s->identifier->symbol] = s;
}

void Parser::AST::note(Parser::Nodes::MemoryDecl *m) {
    if(auto res = memory_decls.find(m->identifier->symbol); res != memory_decls.end()) {
        throw std::runtime_error("Memory redeclaration");
    }
    memory_decls[m->identifier->symbol] = m;
}


Parser::Nodes::FunctionProt *Parser::AST::get_func_prot(const Parser::AST::symbol_t &s) const {
    if(auto res = function_protos.find(s); res == function_protos.end()) {
        return nullptr;
    } else {
        return res->second;
    }
}

Parser::Nodes::FunctionDef *Parser::AST::get_func_def(const Parser::AST::symbol_t &s) const {
    if(auto res = function_defs.find(s); res == function_defs.end()) {
        return nullptr;
    } else {
        return res->second;
    }
}

Parser::Nodes::GlobVariableDecl *Parser::AST::get_glob_var_decl(const Parser::AST::symbol_t &s) const {
    if(auto res = glob_var_decls.find(s); res == glob_var_decls.end()) {
        return nullptr;
    } else {
        return res->second;
    }
}

Parser::Nodes::StructDecl *Parser::AST::get_struct_decl(const Parser::AST::symbol_t &s) const {
    if(auto res = structs_decls.find(s); res == structs_decls.end()) {
        return nullptr;
    } else {
        return res->second;
    }
}

Parser::Nodes::MemoryDecl *Parser::AST::get_memory_decl(const Parser::AST::symbol_t &s) const {
    if(auto res = memory_decls.find(s); res == memory_decls.end()) {
        return nullptr;
    } else {
        return res->second;
    }
}


Parser::AST::const_iterator<Parser::Nodes::FunctionProt*> Parser::AST::iter_func_prot() const {
    return glib::collections::CollectionConstIter(function_protos);
}

Parser::AST::const_iterator<Parser::Nodes::FunctionDef*> Parser::AST::iter_func_def() const {
    return glib::collections::CollectionConstIter(function_defs);
}

Parser::AST::const_iterator<Parser::Nodes::GlobVariableDecl*> Parser::AST::iter_glob_var_decl() const {
    return glib::collections::CollectionConstIter(glob_var_decls);
}

Parser::AST::const_iterator<Parser::Nodes::StructDecl*> Parser::AST::iter_struct_decl() const {
    return glib::collections::CollectionConstIter(structs_decls);
}

Parser::AST::const_iterator<Parser::Nodes::MemoryDecl*> Parser::AST::iter_memory_decl() const {
    return glib::collections::CollectionConstIter(memory_decls);
}