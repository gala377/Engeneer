//
// Created by igor on 14.12.18.
//

#ifndef TKOM2_FILE_H
#define TKOM2_FILE_H

#include <fstream>

#include <lexer/source/base.h>


namespace Lexer::Source {

    class File: public Base {
    public:
        // Opens file under file_path and calls next_char.
        explicit File(std::string file_path);
        // Calls close_file.
        ~File();

        void close_file();

        const char curr_char() override;
        const char next_char() override;
        const char peek() override;

        const uint32_t curr_line() const override;
        const uint32_t curr_in_line_position() const override;

    private:
        std::ifstream _file;

        std::uint32_t _file_line{1};
        std::uint32_t  _in_line_pos{1};

        char _curr_char{'\0'};
        char _next_char{'\0'};
    };
}

#endif //TKOM2_FILE_H
