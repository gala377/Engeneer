//
// Created by igor on 30.09.18.
//

#include <lexer/source.h>

using namespace Lexer;

Source::Source(const std::string& filepath) {

}


const char Source::peek() {
    return 0;
}

const char operator<<(char& ch, Source& s) {
    return ch;
}