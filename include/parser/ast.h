//
// Created by igor on 30.12.18.
//

#ifndef TKOM2_AST_H
#define TKOM2_AST_H

#include <map>

#include <parser/nodes/concrete.h>
#include <parser/nodes/base.h>
#include <parser/visitor.h>

#include <glib/collections.h>

namespace Parser {

    class AST {
    public:
        AST() = default;

        Nodes::BaseParent& root();
        void accept(Visitor &v);

        using symbol_t = std::string;

        template <typename T>
        using symbol_map = std::map<symbol_t, T>;

        void note(Nodes::FunctionProt* f);
        void note(Nodes::FunctionDef* f);
        void note(Nodes::GlobVariableDecl* v);
        void note(Nodes::StructDecl* s);

        Nodes::FunctionProt* get_func_prot(const symbol_t& s) const;
        Nodes::FunctionDef* get_func_def(const symbol_t& s) const;
        Nodes::GlobVariableDecl* get_glob_var_decl(const symbol_t& s) const;
        Nodes::StructDecl* get_struct_decl(const symbol_t& s) const;

        template <typename T>
        using const_iterator = typename glib::collections::CollectionConstIter<symbol_map<T>>;

        const_iterator<Nodes::FunctionProt*> iter_func_prot() const;
        const_iterator<Nodes::FunctionDef*> iter_func_def() const;
        const_iterator<Nodes::GlobVariableDecl*> iter_glob_var_decl() const;
        const_iterator<Nodes::StructDecl*> iter_struct_decl() const;
    private:
        Nodes::Program _root;


        symbol_map<Nodes::FunctionProt*> function_protos;
        symbol_map<Nodes::FunctionDef*> function_defs;
        symbol_map<Nodes::GlobVariableDecl*> glob_var_decls;
        symbol_map<Nodes::StructDecl*> structs_decls;
    };
}

#endif //TKOM2_AST_H
