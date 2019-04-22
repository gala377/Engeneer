//
// Created by igor on 30.09.18.
//

#ifndef TKOM2_SOURCE_H
#define TKOM2_SOURCE_H

#include <tuple>
#include <memory>

namespace Lexer::Source {

    class Base {
    public:

        class Pointer {
        public:
            Pointer(const Base& source): source(source) {};
            virtual ~Pointer() = default;

            virtual std::string source_around(std::uint32_t size = 10) const {
                return source.source_around(*this, size);
            }

        protected:
            const Base& source;
        };

        typedef Pointer pointer;
        typedef const Pointer const_pointer;

        // Returns current character.
        // It's the one before the position of the next character to read.
        virtual const char curr_char() = 0;
        // Proceeds in the source one character at a time.
        // Modifies next character to read position.
        // Returns \0 on eof. todo should ot be like this?
        virtual const char next_char() = 0;

        // Returns character after the current one.
        // Doesn't change next character read position and line.
        // Returns \0 on eof. todo should it be like this?
        virtual const char peek() = 0;

        // Line of the next character to read from the source.
        virtual const std::uint32_t curr_line() const = 0;
        // Position in line of the next character to read from the source.
        virtual const std::uint32_t curr_in_line_position() const = 0;

        // Position of the next character to read from the source.
        virtual const std::tuple<std::uint32_t, std::uint32_t> curr_source_position() const;

        // Returns the name of the source
        virtual const char* name() const = 0;

        virtual const_pointer& current_pointer() = 0;
        virtual std::string source_around(const_pointer& p, std::uint32_t size = 10) const = 0;
    };

    // Assigns current char to ch.
    // Then calls next_char on s.
    void operator<<(char& ch, Base& s);
}

#endif //TKOM2_SOURCE_H
