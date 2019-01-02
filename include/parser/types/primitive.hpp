#ifndef TKOM2_PARSER_HELPERS_PRIMITIVE_TYPES_H
#define TKOM2_PARSER_HELPERS_PRIMITIVE_TYPES_H

#include <string>
#include <variant>

#include <parser/types/storage.hpp>

namespace Parser::Types {

    template <typename ...Args>
    struct PrimitiveTypes {
        typedef typename std::variant<Args...> type;

    };

    template <typename ...Args>
    struct PrimitiveTypes<Storage<Args...>> {
        typedef typename std::variant<Args...> type;
    };

    typedef Storage<int, float, bool, std::string> primitive_storage_t;
    typedef PrimitiveTypes<primitive_storage_t>::type primitive_t; 
    
}

#endif