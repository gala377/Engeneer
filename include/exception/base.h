//
// Created by rafalgal on 26.02.19.
//

#ifndef TKOM2_EXCEPTION_BASE_H
#define TKOM2_EXCEPTION_BASE_H

#include <string>
#include <exception>

#include <boost/format.hpp>
#include <lexer/token.h>


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
            const std::string& mess);
    };

    class BaseSyntax: public  BaseFilePositioned {
    public:
        BaseSyntax(
            std::string file,
            std::uint32_t line,
            std::uint32_t in_line,
            const std::string& mess);

        BaseSyntax(const Lexer::Token& tok, const std::string& mess);
    };
}

#endif //TKOM2_BASE_H
