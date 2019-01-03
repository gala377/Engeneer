//
// Created by igor on 15.12.18.
//

#ifndef TKOM2_LEXER_H
#define TKOM2_LEXER_H

#include <functional>
#include <map>
#include <utility>

#include <lexer/source/base.h>
#include <lexer/token.h>

namespace Lexer {

    class Lexer {
    public:

        explicit Lexer(Source::Base& source);

        const Token curr_token() const;
        const Token peek() const;

        const Token next_token();

        std::tuple<std::uint32_t, std::uint32_t> in_source_pos() const;

    private:
        enum class TokenAssemblerId {
            Identifier,
            Number,
            Operator,
            String,
            //Char,
            Blank,
            Eof,
        };
        using TokenAssembler = std::function<const Token(char)>;
        using AssemblersMap = std::map<TokenAssemblerId, TokenAssembler>;
        using SymbolMap = std::map<std::string, Token::Id>;

        static const SymbolMap _KEYWORDS;
        static const SymbolMap _OPERATORS;

        Source::Base& _source;

        Token _curr_token;
        Token _next_token;

        AssemblersMap _token_assemblers;

        TokenAssemblerId char_to_assembler_id(const char& ch) const;

        Token _process_blank_char(char ch);
        Token _process_numeric(char ch);
        Token _process_identifier(char ch);
        Token _process_operator(char ch);
        Token _process_string(char ch);
        Token _process_eof(char ch);

        std::string _assemble_numeric(char ch);
        std::string _assemble_identifier(char ch);
        std::string _assemble_operator(char ch);
        std::string _assemble_string(char ch);

    };

}

#endif //TKOM2_LEXER_H