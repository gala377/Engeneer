//
// Created by rafalgal on 26.02.19.
//

#ifndef TKOM2_HANDLER_H
#define TKOM2_HANDLER_H

#include <vector>
#include <memory>

#include <exception/base.h>

namespace Exception {

    class Handler {
    public:
        class Abort: std::exception {
        public:
            explicit Abort(Handler& h);
            const char *what() const noexcept override;

        private:
            std::string _trace;
        };

        void error(std::unique_ptr<Exception::Base>&& e);
        void abort(std::unique_ptr<Exception::Base>&& e);
    protected:
        std::vector<std::unique_ptr<Exception::Base>> _excp;

    };

    static Handler default_handler;

}

#endif //TKOM2_HANDLER_H
