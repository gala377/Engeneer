//
// Created by igor on 15.12.18.
//

#ifndef TKOM2_LEXER_H
#define TKOM2_LEXER_H

#include <lexer/source/base.h>

namespace Lexer {

    class Lexer {
    public:
        explicit Lexer(Source::Base& source);


    private:
        Source::Base& source;
    };

}

#endif //TKOM2_LEXER_H
