//
// Created by igor on 14.12.18.
//

#include "lexer/source/base.h"
#include <lexer/source/file.h>

Lexer::Source::File::File(const std::string& file_path): _file_path(file_path) {
    _file.open(file_path);
    if(_file.bad()) {
        throw std::runtime_error("lexer could not open the file");
    }

    next_char();
}

Lexer::Source::File::~File() {
    close_file();
}

void Lexer::Source::File::close_file() {
    _file.close();
}


const char Lexer::Source::File::curr_char() {
    return _curr_char;
}

const char Lexer::Source::File::next_char() {
    if(_next_char != '\0') {
        _curr_char = std::exchange(_next_char, '\0');
    } else {
        _file >> std::noskipws >> _curr_char;
    }

    if(_file.eof()) {
        _curr_char = '\0';
    } else {
        if (_curr_char == '\n') {
            ++_file_line;
            _in_line_pos = 1;
        } else {
            ++_in_line_pos;
        }
    }

    return _curr_char;

}

const char Lexer::Source::File::peek() {
    if(_next_char == '\0') {
        _file >> std::noskipws >> _next_char;
    }
    if(_file.eof()) {
        _next_char = '\0';
    }
    return _next_char;
}

const uint32_t Lexer::Source::File::curr_line() const {
    return _file_line;
}

const uint32_t Lexer::Source::File::curr_in_line_position() const {
    return _in_line_pos;
}

const char *Lexer::Source::File::name() const {
    return _file_path.c_str();
}

Lexer::Source::Base::const_pointer& Lexer::Source::File::current_pointer() {
    auto ptr = FilePointer(_file.tellg(), *this);
    _ptrs.emplace_back(std::move(ptr));
    return _ptrs.back();
}

std::string Lexer::Source::File::source_around(Base::const_pointer &p, uint32_t size) const {
    return source_around(dynamic_cast<const_pointer&>(p), size);
}

std::string Lexer::Source::File::source_around(const_pointer &p, uint32_t size) const {
    auto last_pos = _file.tellg();
    auto pos = p.offset - size;
    pos = pos < 0 ? 0 : pos;
    _file.seekg(pos);
    auto buffer = std::vector<char>(size*2 + 1, 0);
    _file.read(&buffer[0], size*2);
    _file.seekg(last_pos);
    return std::string{buffer.begin(), buffer.end()};
    
}