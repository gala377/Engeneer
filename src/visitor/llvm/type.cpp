//
// Created by igor on 20.03.19.
//

#include <visitor/llvm/type.h>


llvm::Type* Visitor::LLVM::Type::to_llvm(const Parser::Types::BaseType &type, llvm::LLVMContext& c, Compiler::str_map_t& structs) {
    if(auto t = try_as_simple(type, c, structs); t) {
        return t;
    }
    if(auto t = try_as_complex(type, c, structs); t) {
        return t;
    }
    if(auto t = try_as_array(type, c, structs); t) {
        return t;
    }
    if(auto t = try_as_function(type, c, structs); t) {
        return t;
    }
    return nullptr;
}

llvm::Type *Visitor::LLVM::Type::try_as_simple(const Parser::Types::BaseType &type, llvm::LLVMContext& c, Compiler::str_map_t& structs) {
    try {
        const auto &primary = dynamic_cast<const Parser::Types::SimpleType&>(type);
        if(const auto it = _type_handlers.find(primary.ident->symbol); it != _type_handlers.end()) {
            return it->second(type, c);
        }
        if(const auto it = structs.find(primary.ident->symbol); it != structs.end()) {
            return it->second.llvm_str;
        }
        throw std::runtime_error(std::string{"Unknown type: "} + primary.identifier().symbol);
    } catch (std::bad_cast&) {
        return nullptr;
    }
}

llvm::Type *Visitor::LLVM::Type::try_as_array(const Parser::Types::BaseType &type, llvm::LLVMContext& c, Compiler::str_map_t& structs) {
    try {
        const auto &array = dynamic_cast<const Parser::Types::ArrayType&>(type);
        return llvm::ArrayType::get(to_llvm(*array.underlying_type, c, structs), array.size);
    } catch (std::bad_cast&) {
        return nullptr;
    }
}

llvm::Type *Visitor::LLVM::Type::try_as_complex(const Parser::Types::BaseType &type, llvm::LLVMContext& c, Compiler::str_map_t& structs) {
    try {
        const auto &complex = dynamic_cast<const Parser::Types::ComplexType&>(type);
        if(complex.is_ptr) {
            return llvm::PointerType::get(to_llvm(*complex.underlying_type, c, structs), 0);
        }
    } catch (std::bad_cast&) {
        return nullptr;
    }
    return nullptr;
}

llvm::Type *Visitor::LLVM::Type::try_as_function(const Parser::Types::BaseType &type, llvm::LLVMContext &c, Compiler::str_map_t &structs) {
    try {
        const auto &func = dynamic_cast<const Parser::Types::FunctionType&>(type);
        std::vector<llvm::Type*> args;
        for(auto& arg: func.argument_types) {
            args.emplace_back(to_llvm(*arg, c, structs));
        }
        return llvm::PointerType::get(
            llvm::FunctionType::get(to_llvm(*func.return_type, c, structs), args, false),
            0);
    } catch (std::bad_cast&) {
        return nullptr;
    }
}

std::uint32_t Visitor::LLVM::Type::float_size(llvm::Type *t) {
    if(t->isHalfTy()) {
        return 16;
    }
    if(t->isFloatTy()) {
        return 32;
    }
    if(t->isDoubleTy()) {
        return 64;
    }
    if(t->isFP128Ty()) {
        return 128;
    }
    throw std::runtime_error("Unknown float type");
}

std::uint32_t Visitor::LLVM::Type::int_size(llvm::Type *t) {
    return t->getIntegerBitWidth();
}