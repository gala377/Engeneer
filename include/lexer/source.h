//
// Created by igor on 30.09.18.
//

#ifndef TKOM2_SOURCE_H
#define TKOM2_SOURCE_H

#include <istream>

namespace Lexer {

    class Source {
    public:
        Source(const std::string& filepath);

        const char peek();
    };

    const char operator<<(char& ch, Source& s);
}

#endif //TKOM2_SOURCE_H
