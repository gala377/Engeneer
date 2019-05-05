
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
            put(1);
            return 0;
        }
    )"};
    check_output(in, "B");
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

BOOST_AUTO_TEST_CASE(struct_access) {
    std::string in{R"(
        struct A {
            a i32;
            b f32;
            d i32;
        }

        i32 main() {
            let a A;
            a.a = 0;
            a.b = 1.0;
            a.d = 1;
            put(a.d);
            return 0; 
        }
    )"};
    check_output(in, "B");
}

BOOST_AUTO_TEST_CASE(struct_indexing) {
    std::string in{R"(
        struct A {
            a i32;
            b f32;
            d i32;
        }

        i32 main() {
            let a A;
            a.a = 0;
            a.b = 1.0;
            a.d = 1;
            put(a[2]);
            return 0; 
        }
    )"};
    check_output(in, "B");
}

// BOOST_AUTO_TEST_CASE(struct_indexing_past_limits) {
//     std::string in{R"(
//         struct A {
//             a i32;
//             b f32;
//             d i32;
//         }

//         i32 main() {
//             let a A;
//             a.a = 0;
//             a.b = 1.0;
//             a.d = 1;
//             put(a[4]);
//             return 0; 
//         }
//     )"};
//     check_output(in, "B");
// }

// BOOST_AUTO_TEST_CASE(struct_indexing_with_expr) {
//     std::string in{R"(
//         struct A {
//             a i32;
//             b f32;
//             d i32;
//         }

//         i32 main() {
//             let a A;
//             a.a = 0;
//             a.b = 1.0;
//             a.d = 1;
//             put(a[a.d * 2]);
//             return 0; 
//         }
//     )"};
//     check_output(in, "B");
// }

BOOST_AUTO_TEST_CASE(struct_ptr_indexing) {
    std::string in{R"(
        struct A {
            a i32;
            b f32;
            d i32;
        }

        i32 main() {
            let b A;
            let a &A = &b;
            a.a = 0;
            a.b = 1.0;
            a.d = 1;
            put(a[2]);
            return 0; 
        }
    )"};
    check_output(in, "B");
}

BOOST_AUTO_TEST_CASE(acessing_struct_fields_in_array) {
    std::string in{R"(
        struct A {
            a i32;
        }

        [10]A data() {
            let a [10]A; 
            let i i32 = 0;
            while i < 10 {
                let b A; 
                b.a = i;
                a[i] = b;
                i = i +1;
            }
            return a;
        } 

        i32 main() {
            let a [10]A = data();
            let i i32 = 0;
            while i < 10 {
                put(a[i].a);
                i = i +1;
            }
            return 0;
        }
    )"};
    check_output(in, "ABCDEFGHIJ");
}

BOOST_AUTO_TEST_CASE(casting_integer_to_float) {
    std::string in{R"(
        i32 main() {
            let a i32 = 1;
            let b f32 = a as f32;
            let c i32 = b as i32;
            put(c);
            return 0; 
        }
    )"};
    check_output(in, "B");
}

// BOOST_AUTO_TEST_CASE(casting_float_to_integer) {
//     std::string in{R"(
//         i32 main() {
//             let a f32 = 2.0;
//             let b i32 = a as i32; 
//             put(b);
//             return 0; 
//         }
//     )"};
//     check_output(in, "B");
// }

BOOST_AUTO_TEST_CASE(global_variables) {
    std::string in{R"(
    let a i32 = 1;
    i32 main() {
        put(a);
        return 0;
    }
    )"};
    check_output(in, "B");
}

BOOST_AUTO_TEST_CASE(global_variables_after_func_def) {
    std::string in{R"(
    i32 main() {
        put(a);
        return 0;
    }

    let a i32 = 1;
    )"};
    check_output(in, "B");
}

BOOST_AUTO_TEST_CASE(wrapping_structs_works_as_intended) {
    std::string in{R"(
        struct A {
            v i32;
            void put() {
                put(v);
            }
        }

        struct B wraps A {
            void inc() {
                A.v = A.v + 1;
            }
        }

        i32 main() {
            let b B;
            b.inc();
            b.put();
            return 0;
        }
    )"};
    check_output(in, "B");
}

BOOST_AUTO_TEST_CASE(wrapping_multiple_structs_works_as_intended) {
    std::string in{R"(
        struct A {
            v i32;
            i32 get_a() {
                return v;
            }
        }

        struct B  {
            v i32;
            i32 get_b() {
                return v;
            }
        }

        struct C wraps B, A {}
        i32 main() {
            let c C;
            c.A.v = 1;
            c.B.v = 2;
            put(c.get_a());
            put(c.get_b());
            return 0;
        }
    )"};
    check_output(in, "BC");
}

BOOST_AUTO_TEST_CASE(wrapping_multiple_structs_with_conflicts_works_as_intended) {
    std::string in{R"(
        struct A {
            v i32;
            i32 get_v() {
                return v;
            }
        }

        struct B {
            v i32;
            i32 get_v() {
                return v;
            }
        }

        struct C wraps B, A {}
        i32 main() {
            let c C;
            c.A.v = 1;
            c.B.v = 2;
            put(c.get_v());
            return 0;
        }
    )"};
    check_output(in, "C");
}

BOOST_AUTO_TEST_CASE(at_statement_with_dynamic_alloc_and_primitive_type) {
    std::string in{R"(
        any malloc(_ i32);
        void free(_ any);

        memory Heap {

            any dynamic_alloc(size i64) {
                put(size);
                return malloc(size);
            }

            void free(ptr any) {
                free(ptr);
            }
        }

        i32 main() {
            @Heap let a i32 = 1;        
            a = a + 1;
            put(a);
            return 0;
        }
    )"};
    check_output(in, "EC");
}

BOOST_AUTO_TEST_CASE(at_statement_with_dynamic_alloc_and_struct_type) {
    std::string in{R"(
        any malloc(_ i32);
        void free(_ any);

        struct A {
            a i32;
            b i64;
        }

        memory Heap {
            any dynamic_alloc(size i64) {
                put(size);
                return malloc(size);
            }

            void free(ptr any) {
                free(ptr);
            }
        }

        i32 main() {
            @Heap let a A;        
            a.b = 2;
            a.b = a.b + 1;
            put(a.b);
            return 0;
        }
    )"};
    check_output(in, "QD");
}

BOOST_AUTO_TEST_CASE(recursive_struct_compiles_with_ptr) {
    std::string in{R"(
        struct A {
            a &A;
        }

        i32 main() {
            let a A;
            a.a = &a;
            put(1);
            return 0;
        }
    )"};
    check_output(in, "B");
}

// BOOST_AUTO_TEST_CASE(recursive_struct_dsnt_compiles_without_ptr) {
//     std::string in{R"(
// struct A {
//     a A;
// }

// i32 main() {
//     let a A;
//     a.a = a;
//     put(1);
//     return 0;
// }
// )"};
//     check_output(in, "B");
// }

BOOST_AUTO_TEST_CASE(return_void_from_the_function) {
    std::string in{R"(
        void ret_void(a i32) {
            if a > 2 {
                put(1);
                return;
            }
        }

        i32 main() {
            ret_void(10);
            return 0;
        }
    )"};
    check_output(in, "B");
}

BOOST_AUTO_TEST_CASE(unimportat_func_args_identifiers) {
    std::string in{R"(
        i32 add(_1 i32, _2 i32) {
            return _1 + _2;
        }

        i32 main() {
            put(add(1, 0));
            return 0;
        }
    )"};
    check_output(in, "B");
}

BOOST_AUTO_TEST_CASE(assignment_expr) {
    std::string in{R"(
        i32 main() {
            let a i32;
            let b i32 = (a = 1) + 1
            put(b); put(a);
            return 0;
        }
    )"};
    check_output(in, "CB");
}

BOOST_AUTO_TEST_SUITE_END()