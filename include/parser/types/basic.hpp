#ifndef TKOM2_PARSER_HELPERS_BASIC_TYPES_H
#define TKOM2_PARSER_HELPERS_BASIC_TYPES_H

#include <variant>
#include <map>

#include <parser/types/storage.hpp>
#include <parser/types/primitive.hpp>


namespace Parser::Types {

    struct StructVal;

    template <typename T, typename ...Args>
    struct Basic {
        typedef std::variant<T, Args...> type;
    };

    template <typename T, typename ...Args>
    struct Basic<T, Storage<Args>...> {
        typedef std::variant<T, Args...> type; 
    };

    typedef Basic<StructVal, primitive_storage_t>::type basic_t;

    struct StructVal { 
        std::map<std::string, basic_t> values;  
    };

    typedef StructVal struct_t;

}

#endif