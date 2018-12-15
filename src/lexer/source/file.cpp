//
// Created by igor on 14.12.18.
//

#include <lexer/source/file.h>

Lexer::Source::File::File(std::string file_path) {
    _file.open(file_path);
    if(_file.bad()) {
        // todo custom error
        throw std::runtime_error("Lexer could not open the file");
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