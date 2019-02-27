//
// Created by igor on 27.02.19.
//

#include <exception/handling_mixin.h>

Exception::HandlingMixin::HandlingMixin(Exception::Handler &h): _excp_handler(h) {}

const Exception::Handler& Exception::HandlingMixin::excp_handler() const {
    return _excp_handler;
}
