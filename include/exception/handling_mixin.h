//
// Created by igor on 27.02.19.
//

#ifndef TKOM2_HANDLING_MIXIN_H
#define TKOM2_HANDLING_MIXIN_H

#include <exception/handler.h>

namespace Exception {

    class HandlingMixin {
    public:
        HandlingMixin(Exception::Handler& h = Exception::default_handler);


        const Exception::Handler& excp_handler() const;

    protected:
        Exception::Handler& _excp_handler;

        template <typename T, typename ...Args>
        void error(Args&&... args) {
            _excp_handler.error(std::make_unique<T>(std::forward<Args>(args)...));
        }

        template <typename T, typename ...Args>
        void abort(Args&&... args){
            _excp_handler.abort(std::make_unique<T>(std::forward<Args>(args)...));
        }
    };
}

#endif //TKOM2_HANDLING_MIXIN_H
