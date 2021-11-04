#pragma once

#include "parser/handlers/base_handler.hpp"

namespace parser {

class BranchRootHandler : public BaseHandler {
    int nestingLevel;

  public:
    virtual void run(ParserState &state) override;
    virtual void reset() override;
};

} // namespace parser
