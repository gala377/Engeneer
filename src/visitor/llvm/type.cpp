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


Visitor::LLVM::Type::TypeDeduction::TypeDeduction(
        const Compiler::func_map_t& functions,
        const Compiler::var_map_t& variables):
        _functions(functions), _variables(variables), Base() {}

void Visitor::LLVM::Type::TypeDeduction::visit(const Parser::Nodes::LogicalOrExpr &node) {
    Base::visit(node);
}

void Visitor::LLVM::Type::TypeDeduction::visit(const Parser::Nodes::LogicalAndExpr &node) {
    Base::visit(node);
}

void Visitor::LLVM::Type::TypeDeduction::visit(const Parser::Nodes::InclusiveOrExpr &node) {
    Base::visit(node);
}

void Visitor::LLVM::Type::TypeDeduction::visit(const Parser::Nodes::ExclusiveOrExpr &node) {
    Base::visit(node);
}

void Visitor::LLVM::Type::TypeDeduction::visit(const Parser::Nodes::AndExpr &node) {
    Base::visit(node);
}

void Visitor::LLVM::Type::TypeDeduction::visit(const Parser::Nodes::EqualityExpr &node) {
    Base::visit(node);
}

void Visitor::LLVM::Type::TypeDeduction::visit(const Parser::Nodes::RelationalExpr &node) {
    Base::visit(node);
}

void Visitor::LLVM::Type::TypeDeduction::visit(const Parser::Nodes::ShiftExpr &node) {
    Base::visit(node);
}


void Visitor::LLVM::Type::TypeDeduction::visit(const Parser::Nodes::AdditiveExpr &node) {
    Base::visit(node);
}

void Visitor::LLVM::Type::TypeDeduction::visit(const Parser::Nodes::MultiplicativeExpr &node) {
    Base::visit(node);
}


void Visitor::LLVM::Type::TypeDeduction::visit(const Parser::Nodes::NegativeExpr &node) {
    Base::visit(node);
}

void Visitor::LLVM::Type::TypeDeduction::visit(const Parser::Nodes::NegationExpr &node) {
    Base::visit(node);
}


void Visitor::LLVM::Type::TypeDeduction::visit(const Parser::Nodes::CallExpr &node) {
    Base::visit(node);
}

void Visitor::LLVM::Type::TypeDeduction::visit(const Parser::Nodes::IndexExpr &node) {
    Base::visit(node);
}

void Visitor::LLVM::Type::TypeDeduction::visit(const Parser::Nodes::AccessExpr &node) {
    Base::visit(node);
}

void Visitor::LLVM::Type::TypeDeduction::visit(const Parser::Nodes::Identifier &node) {
    auto it = _variables.find(node.symbol);
    if(it == _variables.end()) {
        throw std::runtime_error("Variable doesn't exist: " + node.symbol);
    }
    auto t_copy = it->second.var->type.get();
    // todo what todo what todo
}

void Visitor::LLVM::Type::TypeDeduction::visit(const Parser::Nodes::ParenthesisExpr &node) {
    node.expr->accept(*this);
}

void Visitor::LLVM::Type::TypeDeduction::visit(const Parser::Nodes::IntConstant &node) {
    _owned_type = std::make_unique<Parser::Types::SimpleType>(
            std::make_unique<Parser::Nodes::Identifier>(i32_id));
}

void Visitor::LLVM::Type::TypeDeduction::visit(const Parser::Nodes::StringConstant &node) {
    _owned_type = std::make_unique<Parser::Types::ArrayType>(
            node.value.size(),
            std::make_unique<Parser::Types::SimpleType>(
                    std::make_unique<Parser::Nodes::Identifier>(byte_id)));
}

void Visitor::LLVM::Type::TypeDeduction::visit(const Parser::Nodes::FloatConstant &node) {
    _owned_type = std::make_unique<Parser::Types::SimpleType>(
            std::make_unique<Parser::Nodes::Identifier>(f64_id));

}
