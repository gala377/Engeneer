//
// Created by igor on 14.12.18.
//

#ifndef TKOM2_STRING_H
#define TKOM2_STRING_H

#include <lexer/source/base.h>

namespace Lexer::Source {

    class String: public Base {
    public:
        explicit String(const std::string& raw);
        explicit String(std::string&& raw);

        const char curr_char() override;
        const char next_char() override;

        const char peek() override;

        const uint32_t curr_line() const override;
        const uint32_t curr_in_line_position() const override;

        const char *name() const override;

    private:
        std::string _raw;
        std::int32_t  _curr_index = -1;

        std::uint32_t _curr_line = 1;
        std::uint32_t _in_line_pos = 1;

    };

}

#endif //TKOM2_STRING_H