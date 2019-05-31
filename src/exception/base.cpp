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
    Lexer::Source::Base::const_pointer& ptr,
    const std::string& mess): BaseFormatted("[%1% %2%:%3%] %4%\n %5%", file, line, in_line, mess, ptr.source_around(25)) {}

Exception::BaseSyntax::BaseSyntax(const Lexer::Token &tok, const std::string &mess):
    BaseSyntax {
        tok.span.source_name,
        tok.span.line,
        tok.span.beg_pos,
        tok.span.ptr,
        mess
    } {}

Exception::BaseCompilation::BaseCompilation(const Parser::Nodes::Base& node, const std::string& mess):
    BaseCompilation {
        node.span().source_name,
        node.span().line,
        node.span().beg_pos,
        node.span().ptr,
        mess
    } {}

Exception::BaseCompilation::BaseCompilation(const Lexer::Token::Span& span, const std::string& mess): 
    BaseCompilation {
        span.source_name,
        span.line,
        span.beg_pos,
        span.ptr,
        mess
    } {}
