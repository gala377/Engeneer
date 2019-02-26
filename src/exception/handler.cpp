//
// Created by rafalgal on 26.02.19.
//

#include <exception/handler.h>

void Exception::Handler::error(std::unique_ptr<Exception::Base> &&e) {
    _excp.emplace_back(std::move(e));
}

void Exception::Handler::abort(std::unique_ptr<Exception::Base> &&e) {
    error(std::move(e));
    throw Abort(*this);
}


Exception::Handler::Abort::Abort(const Exception::Handler &h) {
    for(const auto& e: h._excp) {
        _trace += e->str() + "\n\n";
    }
}

const char *Exception::Handler::Abort::what() const noexcept {
    return _trace.c_str();
}

void Exception::Handler::throw_if_able() const {
    if(!_excp.empty()) {
        throw Abort(*this);
    }
}
