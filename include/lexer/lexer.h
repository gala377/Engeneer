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

#include <exception/handling_mixin.h>

namespace Lexer {

    class Lexer: public Exception::HandlingMixin {
    public:

        explicit Lexer(Source::Base& source);
        explicit Lexer(Source::Base& source, Exception::Handler& h);


        const Token curr_token() const;
        const Token peek() const;

        const Token next_token();

        const Token make_token(Token::Id id, const std::string& symbol);

        std::tuple<std::uint32_t, std::uint32_t> in_source_pos() const;
    private:
        enum class TokenAssemblerId {
            Identifier,
            Number,
            Operator,
            String,
            Blank,
            Eof,
            Comment,
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

        void _init_token_assemblers();
        void _init_curr_token();

        TokenAssemblerId char_to_assembler_id(const char& ch);

        Token _process_blank_char(char ch);
        Token _process_numeric(char ch);
        Token _process_identifier(char ch);
        Token _process_operator(char ch);
        Token _process_string(char ch);
        Token _process_eof(char ch);
        Token _process_comment(char ch);

        std::tuple<bool, std::string> _assemble_numeric(char ch);
        std::string _assemble_identifier(char ch);
        std::string _assemble_operator(char ch);
        std::string _assemble_string(char ch);
        std::string _assemble_comment(char ch);

        template <typename T, typename Arg>
        void abort_w(Arg&& mess) {
            abort<T>(
                _source.name(),
                _source.curr_line(),
                _source.curr_in_line_position(),
                _source.current_pointer(),
                std::forward<Arg>(mess));
        }

    };

}

#endif //TKOM2_LEXER_H
