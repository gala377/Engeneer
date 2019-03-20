//
// Created by igor on 20.03.19.
//

#include <visitor/llvm/type.h>


llvm::Type* Visitor::LLVM::Type::to_llvm(const Parser::Types::BaseType &type, llvm::LLVMContext& c) {
    if(auto t = try_as_simple(type, c); t) {
        return t;
    }
    if(auto t = try_as_complex(type, c); t) {
        return t;
    }
    if(auto t = try_as_array(type, c); t) {
        return t;
    }
    throw std::runtime_error("Unknown type: " + type.identifier().symbol);
}

llvm::Type *Visitor::LLVM::Type::try_as_simple(const Parser::Types::BaseType &type, llvm::LLVMContext& c) {
    try {
        const auto &primary = dynamic_cast<const Parser::Types::SimpleType&>(type);
        if(const auto it = _type_handlers.find(primary.ident->symbol); it != _type_handlers.end()) {
            return it->second(type, c);
        }
        // todo possibly structure
        return nullptr;
    } catch (std::bad_cast&) {
        return nullptr;
    }
}

llvm::Type *Visitor::LLVM::Type::try_as_array(const Parser::Types::BaseType &type, llvm::LLVMContext& c) {
    try {
        const auto &array = dynamic_cast<const Parser::Types::ArrayType&>(type);
        return llvm::ArrayType::get(to_llvm(*array.underlying_type, c), array.size);
    } catch (std::bad_cast&) {
        return nullptr;
    }
}

llvm::Type *Visitor::LLVM::Type::try_as_complex(const Parser::Types::BaseType &type, llvm::LLVMContext& c) {
    try {
        const auto &complex = dynamic_cast<const Parser::Types::ComplexType&>(type);
        if(complex.is_ptr) {
            return llvm::PointerType::get(to_llvm(*complex.underlying_type, c), 0);
        }
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