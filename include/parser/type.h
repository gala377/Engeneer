//
// Created by rafalgal on 07.03.19.
//

#ifndef TKOM2_PARSER_TYPE_H
#define TKOM2_PARSER_TYPE_H

#include <string>
#include <memory>

#include <parser/nodes/concrete.h>

namespace Parser::Types {

    class SimpleType: public BasicType {
    public:
        explicit SimpleType(std::unique_ptr<Nodes::Identifier>&& ident);

        std::unique_ptr<Nodes::Identifier> ident;

        const Nodes::Identifier& identifier() const override;
    };

    class ComplexType: public BasicType {
    public:
        // true if type is const of underlying type
        // if it's ptr as well its a const ptr.
        bool is_const{false};
        // true if type is ptr to underlying type
        bool is_ptr{false};
        // Type to which the modifiers should be applied to
        std::unique_ptr<BasicType> underlying_type{nullptr};

        const Nodes::Identifier& identifier() const override;
    };
}

#endif //TKOM2_TYPE_H

