//
// Created by igor on 14.12.18.
//

#ifndef TKOM2_FILE_H
#define TKOM2_FILE_H

#include <bits/stdint-uintn.h>
#include <fstream>

#include <lexer/source/base.h>
#include <vector>


namespace Lexer::Source {

    class File: public Base {
    public:
        // Opens file under file_path and calls next_char.
        explicit File(const std::string& file_path);
        // Calls close_file.
        ~File();

        void close_file();

        const char curr_char() override;
        const char next_char() override;
        const char peek() override;

        const uint32_t curr_line() const override;
        const uint32_t curr_in_line_position() const override;

        const char *name() const override;

        class FilePointer: public Pointer {
        public:
            FilePointer(uint32_t offset, const File& f): Pointer(f), offset(offset) {} 
            uint32_t offset;
        };

        typedef FilePointer pointer;
        typedef const FilePointer const_pointer;

        Base::const_pointer& current_pointer() override;
        std::string source_around(Base::const_pointer& p, uint32_t size = 10) const override;
        virtual std::string source_around(const_pointer& p, uint32_t size = 10) const;

    private:
        mutable std::ifstream _file;
        std::string _file_path;

        std::uint32_t _file_line{1};
        std::uint32_t  _in_line_pos{1};

        char _curr_char{'\0'};
        char _next_char{'\0'};

        std::vector<FilePointer> _ptrs;
    };
}

#endif //TKOM2_FILE_H
