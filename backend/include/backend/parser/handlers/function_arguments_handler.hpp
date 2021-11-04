#pragma once

#include "parser/handlers/base_handler.hpp"

namespace parser {

class FunctionArgumentsHandler : public BaseHandler {
  public:
    virtual void run(ParserState &state) override;
};

} // namespace parser
