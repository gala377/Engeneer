//
// Created by igor on 15.12.18.
//

#include <lexer/source/string.h>
#include <algorithm>


Lexer::Source::String::String(const std::string &raw): _raw(raw) {
    next_char();
}

Lexer::Source::String::String(std::string &&raw): _raw(raw) {
    next_char();
}

const char Lexer::Source::String::curr_char() {
    return _curr_index < _raw.length() ? _raw[_curr_index] : '\0' ;
}

const char Lexer::Source::String::next_char() {
    if(++_curr_index < _raw.length()) {
        ++_in_line_pos;
        if (_raw[_curr_index] == '\n') {
            ++_curr_line;
            _in_line_pos = 1;
        }
    }
    return curr_char();
}

const char Lexer::Source::String::peek() {
    return (_curr_index + 1) < _raw.length() ? _raw[_curr_index+1] : '\0' ;
}

const uint32_t Lexer::Source::String::curr_line() const {
    return _curr_line;
}

const uint32_t Lexer::Source::String::curr_in_line_position() const {
    return _in_line_pos;
}

const char *Lexer::Source::String::name() const {
    return "str source";
}

Lexer::Source::Base::const_pointer& Lexer::Source::String::current_pointer() {
    _ptrs.emplace_back(StringPointer((uint32_t)std::max(_curr_index, 0), *this));
    return _ptrs.back();
}

std::string Lexer::Source::String::source_around(Lexer::Source::Base::const_pointer &p, uint32_t size) {
    auto ptr = dynamic_cast<const_pointer&>(p);
    return source_around(ptr, size);
}

std::string Lexer::Source::String::source_around(Lexer::Source::String::const_pointer &p, uint32_t size) {
    return _raw.substr(std::min((std::uint32_t)0, p.i - size), size);
}
