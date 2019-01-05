//
// Created by igor on 05.01.19.
//

#ifndef TKOM2_COMBINATORS_H
#define TKOM2_COMBINATORS_H

#include <functional>
#include <vector>

#include <parser/nodes/base.h>

namespace Parser::Combinators {

    // Because of c++ std::function limitations we cannot do it the nice way
    // all the functions below are templated but the parser functions should always
    // be ParserFunc it is just so you cannot make a std::function with lambdas
    // having a unique_ptr moved in them because they are not copyable.
    // So we need those templates to move our lambdas around.

    using parse_res_t = std::unique_ptr<Nodes::Base>;

//    // Returns first success from given parsers
//    // This will not work because of templates
//    template <typename T>
//    auto one_of(const std::vector<T>& parsers) {
//        return [=](auto& l) -> parse_res_t {
//            for (auto &p: parsers) {
//                if (auto res = p(l); res) {
//                    return res;
//                }
//            }
//            return {nullptr} ;
//        };
//    }
//
//
//    // Proceeds lexer if all parsers failed.
//    // This will not work because of templates
//    template <typename T>
//    auto none_of(const std::vector<T>& parsers) {
//        return [=](auto& l) -> parse_res_t  {
//            for (auto &p: parsers) {
//                if (auto res = p(l); res) {
//                    return {nullptr};
//                }
//            }
//            auto tok = l.curr_token;
//            l.next_token();
//            return std::make_unique<Nodes::BaseToken>(tok);
//        };
//    }


    // maps function to parser result if succeeded, then returns it
    template <typename T, typename U>
    auto fmap(const T& p, const U& f) {
        return [=](auto& l) -> std::result_of_t<U> {
            if (auto res = p(l); res) {
                return f(std::move(res));
            }
            return {nullptr};
        };
    }


    // Call p if success calls U with the result and the source.
    // So f should be of type parse_res_t (*)(parse_res_t, Lexer::Lexer&);
    template <typename T, typename U>
    auto bind(const T& p, const U& f) {
        return [=](auto& l) -> std::result_of_t<U> {
            if(auto res = p(l); res) {
                return f(res, l);
            }
            return {nullptr};
        };
    }


    // Runs b if a failed
    template <typename T, typename U>
    auto fallback(const T& a, const U& b) {
        return [=](auto& l) {
            if (auto res = a(l); res) {
                return std::move(res);
            }
            return b(l);
        };
    }


    // Runs a and b in succession. Returning nullptr when any fails
    // If both succeed passes results as arguments for f.
    template <typename T, typename U, typename W>
    auto combine(const T& a, const U& b, const W& f) {
        return [=](auto& l) {
            auto res_a = a(l);
            if (!res_a) {
                return res_a;
            }
            auto res_b = b(l);
            if (!res_b) {
                return res_b;
            }
            return f(std::move(res_a), std::move(res_b));
        };
    }


    // If both succeeds returns only the right value
    template <typename T, typename U>
    auto return_right(const T& a, const U& b) {
        return combine(a, b, [](auto, auto r) -> decltype(r){
            return r;
        });
    }


    // If both succeeds returns only the left value
    template <typename T, typename U>
    auto return_left(const T& a, const U& b) {
        return combine(a, b, [](auto& l, auto) -> decltype(l) {
                    return l;
        });
    }


    // fallback alias
    template <typename T, typename U>
    auto operator|(const T& a, const U& b) {
        return fallback(a, b);
    }


    // return right alias
    template <typename T, typename U>
    auto operator<(const T& a, const U& b) {
        return return_right(a, b);
    }


    // return left alias
    template <typename T, typename U>
    auto operator>(const T& a, const U& b) {
        return return_left(a, b);
    }


    // runs p until it fails. Accumulates results starting from init using f.
    template <typename T, typename U>
    auto many(const T& p, parse_res_t init, const U& f) {
        return [init = std::move(init), p, f](auto& l) mutable -> decltype(init) {
            while (true) {
                auto res = p(l);
                if (!res) {
                    return std::move(init);
                }
                init = std::move(f(std::move(init), std::move(res)));
            };
        };
    }


    // as many but if p faild the first time returns nullptr
    template <typename T, typename U, typename W>
    auto many_once(const T& p, U init, const W& f) {
        return [=, init = std::move(init)](auto& l) mutable -> decltype(init) {
            auto res = p(l);
            if (!res) {
                return {nullptr};
            }
            init = std::move(f(std::move(init), std::move(res)));
            while (true) {
                auto new_res = p(l);
                if (!new_res) {
                    return std::move(init);
                }
                init = std::move(f(std::move(init), std::move(new_res)));
            };
        };
    }


    // Tries to run exactly n times and if fails return nullptr.
    template <typename T, typename U, typename W>
    auto exactly_n(const T& p, std::uint32_t n, U init, const W& f) {
        return [=, init = std::move(init)](auto& l) mutable -> decltype(init) {
            for (std::uint32_t i = 0; i < n; ++i) {
                auto res = p(l);
                if (!res) {
                    return {nullptr};
                }
                init = std::move(f(std::move(init), std::move(res)));
            };
            return std::move(init);
        };
    }


    // runs two parsers in sequence leaving only the left one,
    // and folds result with init using f until one of them they fails.
    template <typename T, typename U, typename W, typename X>
    auto separated_by(const T& p, const U& s, W init, const X& f) {
        return many(p > s, std::move(init), f);
    }

}


#endif //TKOM2_COMBINATORS_H
