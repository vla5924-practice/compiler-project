#pragma once

#include "parser/handlers/base_handler.hpp"

namespace parser {

class ProgramRootHandler : public BaseHandler {
  public:
    virtual void run(ParserState &state) override;
};

} // namespace parser
