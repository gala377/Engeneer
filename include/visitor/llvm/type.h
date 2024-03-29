//
// Created by igor on 20.03.19.
//

#ifndef TKOM2_VISITOR_TYPE_H
#define TKOM2_VISITOR_TYPE_H

#include <llvm/IR/DerivedTypes.h>
#include <parser/visitor.h>

#include <visitor/base.h>
#include <visitor/llvm/compiler.h>
#include <parser/type.h>
#include <parser/nodes/base.h>

#include <llvm/IR/LLVMContext.h>


namespace Visitor::LLVM::Type {
    
        const std::string void_id{"void"};
        const std::string bool_id{"bool"};
        const std::string byte_id{"byte"};
        const std::string i8_id{"i8"};
        const std::string i16_id{"i16"};
        const std::string i32_id{"i32"};
        const std::string i64_id{"i64"};
        const std::string u8_id{"u8"};
        const std::string u16_id{"u16"};
        const std::string u32_id{"u32"};
        const std::string u64_id{"u64"};
        const std::string f32_id{"f32"};
        const std::string f64_id{"f64"};
        const std::string f128_id{"f128"};
        const std::string any_ptr_id{"any"};

        using type_handler = std::function<llvm::Type*(const Parser::Types::BaseType&, llvm::LLVMContext&)>;
        using handlers_map = std::map<std::string, type_handler>;

        llvm::Type* to_llvm(const Parser::Types::BaseType& type, llvm::LLVMContext& c, Compiler::str_map_t& structs);
        llvm::Type* try_as_simple(const Parser::Types::BaseType& type, llvm::LLVMContext& c, Compiler::str_map_t& structs);
        llvm::Type* try_as_array(const Parser::Types::BaseType& type, llvm::LLVMContext& c, Compiler::str_map_t& structs);
        llvm::Type* try_as_complex(const Parser::Types::BaseType& type, llvm::LLVMContext& c, Compiler::str_map_t& structs);
        llvm::Type* try_as_function(const Parser::Types::BaseType& type, llvm::LLVMContext& c, Compiler::str_map_t& structs);

        std::uint32_t float_size(llvm::Type* t);
        std::uint32_t int_size(llvm::Type* t);

        static const handlers_map _type_handlers = {
                {
                        void_id,
                        [](const Parser::Types::BaseType& type, llvm::LLVMContext& context) -> llvm::Type* {
                            return llvm::Type::getVoidTy(context);
                        }
                },
                {
                        bool_id,
                        [](const Parser::Types::BaseType& type, llvm::LLVMContext& context) -> llvm::Type* {
                            return llvm::Type::getInt1Ty(context);
                        }
                },
                {
                        byte_id,
                        [](const Parser::Types::BaseType& type, llvm::LLVMContext& context) -> llvm::Type* {
                            return llvm::Type::getInt8Ty(context);
                        }
                },
                {
                        i8_id,
                        [](const Parser::Types::BaseType& type, llvm::LLVMContext& context) -> llvm::Type* {
                            return llvm::Type::getInt8Ty(context);
                        }
                },
                {
                        i16_id,
                        [](const Parser::Types::BaseType& type, llvm::LLVMContext& context) -> llvm::Type* {
                            return llvm::Type::getInt16Ty(context);
                        }
                },
                {
                        i32_id,
                        [](const Parser::Types::BaseType& type, llvm::LLVMContext& context) -> llvm::Type* {
                            return llvm::Type::getInt32Ty(context);
                        }
                },
                {
                        i64_id,
                        [](const Parser::Types::BaseType& type, llvm::LLVMContext& context) -> llvm::Type* {
                            return llvm::Type::getInt64Ty(context);
                        }
                },
                {
                        f32_id,
                        [](const Parser::Types::BaseType& type, llvm::LLVMContext& context) -> llvm::Type* {
                            return llvm::Type::getFloatTy(context);
                        }
                },
                {
                        f64_id,
                        [](const Parser::Types::BaseType& type, llvm::LLVMContext& context) -> llvm::Type* {
                            return llvm::Type::getDoubleTy(context);
                        }
                },
                {
                        f128_id,
                        [](const Parser::Types::BaseType& type, llvm::LLVMContext& context) -> llvm::Type* {
                            return llvm::Type::getFP128Ty(context);
                        }
                },
                {
                        any_ptr_id,
                        [](const Parser::Types::BaseType& type, llvm::LLVMContext& context) -> llvm::Type* {
                            return llvm::PointerType::getInt8PtrTy(context);
                        }
                },
        };
}

#endif //TKOM2_TYPE_H
