//
// Created by rafalgal on 26.02.19.
//

#ifndef TKOM2_EXCEPTION_BASE_H
#define TKOM2_EXCEPTION_BASE_H

#include <string>
#include <exception>

#include <boost/format.hpp>
#include <lexer/token.h>
#include <lexer/source/base.h>
#include <parser/nodes/base.h>

namespace Exception {

    class Base: public std::exception {
    public:
        explicit Base(const std::string& mess);

        virtual const std::string& str() const;

        const char *what() const noexcept override;
    protected:
        std::string _mess;
    };

    class BaseFormatted: public Base {
    public:
        template <typename ...Args>
        BaseFormatted(const std::string& format, Args... args): Base(""), _fmtr(format) {
            (_fmtr % ... % args);
            _mess = _fmtr.str();
        }
    protected:
        boost::format _fmtr;
    };

    class BaseFilePositioned: public BaseFormatted {
    public:
        BaseFilePositioned(
            std::string file,
            std::uint32_t line,
            std::uint32_t in_line,
            Lexer::Source::Base::const_pointer& ptr,
            const std::string& mess);
    };

    class BaseSyntax: public BaseFilePositioned {
    public:
        using BaseFilePositioned::BaseFilePositioned;

        BaseSyntax(const Lexer::Token& tok, const std::string& mess);
    };

    class BaseCompilation: public BaseFilePositioned {
    public:
        using BaseFilePositioned::BaseFilePositioned;

        BaseCompilation(const Parser::Nodes::Base& node, const std::string& mess);
        BaseCompilation(const Lexer::Token::Span& span, const std::string& mess);
    };
}

#endif //TKOM2_BASE_H
