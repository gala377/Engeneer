#ifndef TKOM2_PARSER_FUNCTIONS_H_
#define TKOM2_PARSER_FUNCTIONS_H_

#include <tuple>
#include <memory>
#include <optional>
#include <functional>
#include <type_traits>

#include <lexer/lexer.h>
#include <parser/nodes/base.h>

namespace Parser::Functions {

    namespace traits {
        template <typename T>
        struct is_checkable {
            static const bool value = false;
        };

        template <typename T>
        struct is_checkable<std::optional<T>> {
            static const bool value = true;
        };

        template <typename T>
        struct is_checkable<std::unique_ptr<T>> {
            static const bool value = true;
        };

        template <typename T>
        using is_checkable_v = typename is_checkable<T>::value;
    }

    using parse_input_t = Lexer::Lexer&;
    template <typename T, typename = typename std::enable_if<traits::is_checkable<T>::value>> 
    using parse_res_t = T;
    
    template <typename T>
    using ParserFunc = std::function<parse_res_t<T>(parse_input_t)>;

    using node_parse_res_t = std::unique_ptr<Nodes::Base>;
    using tok_parse_res_t = std::optional<Lexer::Token>;

    using NodeParserFunc = ParserFunc<node_parse_res_t>;
    using TokenParserFunc = ParserFunc<tok_parse_res_t>;

    // Passes given parser
    template <typename T>
    auto Succes(ParserFunc<T> p) {
        return p;
    }
    // Returns parser that always fail
    auto Fail(NodeParserFunc p) {
        return [](Lexer::Lexer& l) { return std::unique_ptr<Nodes::Base>{nullptr}; };
    }

    auto Fail(TokenParserFunc p) {
        return [](Lexer::Lexer& l) { return std::nullopt; };
    }
    

}

#endif