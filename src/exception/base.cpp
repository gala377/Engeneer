//
// Created by rafalgal on 26.02.19.
//

#include <exception/base.h>

Exception::Base::Base(const std::string& mess): _mess(mess) {}

const std::string& Exception::Base::str() const {
    return _mess;
}

const char *Exception::Base::what() const noexcept {
    return _mess.c_str();
}

Exception::BaseFilePositioned::BaseFilePositioned(
    std::string file,
    std::uint32_t line,
    std::uint32_t in_line,
    std::string mess): BaseFormatted("[%1% %2%:%3%] %4%", file, line, in_line, mess) {}

Exception::BaseSyntax::BaseSyntax(
    std::string file,
    std::uint32_t line,
    std::uint32_t in_line,
    std::string mess): BaseFilePositioned(file, line, in_line, std::string{"Syntax error: "} + mess) {}