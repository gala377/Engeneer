
#include <string>
#include <visitor/llvm/compiler.h>
#include <boost/asio/io_service.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/process/detail/child_decl.hpp>
#include <boost/process/io.hpp>
#include <boost/process/system.hpp>
#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <future>
#include <iostream>

#include <boost/test/unit_test.hpp>
#include <boost/process.hpp>

#include <parser/parser.h>
#include <lexer/source/string.h>
#include <exception/concrete.h>
#include <stdexcept>
#include <visitor/log.h>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

struct Fixture {
    Fixture() = default;
    ~Fixture() {
        fs::remove("./output.o");
        fs::remove("./test_bin");
    }
};

BOOST_FIXTURE_TEST_SUITE(compiler_tests, Fixture)

namespace bp = boost::process;

void compile_to_obj_file(const std::string& source) {
    Lexer::Source::String lex_source{source};
    Parser::Parser parser{lex_source};
    auto ast = parser.parse();
    Visitor::LLVM::Compiler compiler(ast);
    ast.accept(compiler);
}

void compile_to_binary() {
    std::cerr << "Compiling to binary\n";
    auto res = bp::system("clang output.o -o test_bin");
    if(res > 0) {
        BOOST_FAIL("Could not compile object file to binary.");
    }
    std::cerr << "Compiled...\n";
}

std::string add_testing_prelude(const std::string& source) {
    std::string prelude=R"(
i32 putchar(_ i32);
i32 put(ch i32) {
    return putchar(ch + 65);
}
)";
    return prelude + source;
}

void compile_program(const std::string& source) {
    auto enriched_source = add_testing_prelude(source);
    compile_to_obj_file(enriched_source);
    compile_to_binary();
}

std::string get_program_output(const std::string& source) {
    compile_program(source);
    boost::asio::io_service ios;
    std::future<std::string> out;

    std::cerr << "Running test binary\n";
    bp::child prog("./test_bin",
        bp::std_in.close(),
        bp::std_out > out,
        bp::std_err > bp::null,
        ios);
    ios.run();
    std::cerr << "Done...\n";
    auto prog_out = out.get();
    return prog_out;
}

void check_output(const std::string& in, const std::string& out) {
    auto prog_out = get_program_output(in);
    BOOST_CHECK_EQUAL(out, prog_out);
}

BOOST_AUTO_TEST_CASE(printing_simple_integer_const) {
    std::string in{R"(
        i32 main() {
            put(0);
            return 0;
        }
    )"};
    check_output(in, "A");
}

BOOST_AUTO_TEST_CASE(calling_a_method_on_a_struct_ptr) {
    std::string in{R"(
        struct A {
            void foo() {
                put(1);
            }
        }

        i32 main() {
            let a A;
            let b &A = &a;
            b.foo();
            return 0; 
        }
    )"};
    check_output(in, "B");
}

BOOST_AUTO_TEST_CASE(calling_a_method_on_a_struct) {
    std::string in{R"(
        struct A {
            void foo() {
                put(1);
            }
        }

        i32 main() {
            let a A;
            a.foo();
            return 0; 
        }
    )"};
    check_output(in, "B");
}

BOOST_AUTO_TEST_CASE(calling_a_method_on_a_implicit_this) {
    std::string in{R"(
        struct A {
            void foo() {
                put(1);
            }

            void bar() {
                foo();
            }
        }

        i32 main() {
            let a A;
            a.bar();
            return 0; 
        }
    )"};
    check_output(in, "B");
}

BOOST_AUTO_TEST_CASE(accessing_array_on_struct) {
    std::string in{R"(
        struct A {
            array [10]i32;
        }

        A get_a() {
            let a A;
            let i i32 = 0;
            while i < 10 {
                a.array[i] = i;
                i = i + 1;
            }
            return a;
        }

        i32 main() {
            let a A = get_a();
            put(a.array[1]);
            return 0; 
        }
    )"};
    check_output(in, "B");
}

BOOST_AUTO_TEST_SUITE_END()