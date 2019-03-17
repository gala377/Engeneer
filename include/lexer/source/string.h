//
// Created by igor on 14.12.18.
//

#ifndef TKOM2_STRING_H
#define TKOM2_STRING_H

#include <lexer/source/base.h>
#include <vector>

namespace Lexer::Source {

    class String: public Base {
    public:
        class StringPointer: public Pointer {
        public:
            StringPointer(uint32_t i, String& s): Pointer(s), i(i) {}

            uint32_t i;
        };

        typedef StringPointer pointer;
        typedef const StringPointer const_pointer;

        explicit String(const std::string& raw);
        explicit String(std::string&& raw);

        const char curr_char() override;
        const char next_char() override;

        const char peek() override;

        const uint32_t curr_line() const override;
        const uint32_t curr_in_line_position() const override;

        const char *name() const override;

        Base::const_pointer& current_pointer() override;
        std::string source_around(Base::const_pointer &p, uint32_t size = 10) const override;
        virtual std::string source_around(const_pointer &p, uint32_t size = 10) const;
    private:
        std::string _raw;
        std::int32_t  _curr_index = -1;

        std::uint32_t _curr_line = 1;
        std::uint32_t _in_line_pos = 1;

        std::vector<StringPointer> _ptrs;
    };

}

#endif //TKOM2_STRING_H