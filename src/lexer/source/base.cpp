//
// Created by igor on 14.12.18.
//

#include <lexer/source/base.h>

const std::tuple<std::uint32_t, std::uint32_t> Lexer::Source::Base::curr_source_position() const {
    return std::make_tuple(
            curr_line(),
            curr_in_line_position());
}

void Lexer::Source::operator<<(char& ch, Lexer::Source::Base& s) {
    ch = s.curr_char();
    s.next_char();
}