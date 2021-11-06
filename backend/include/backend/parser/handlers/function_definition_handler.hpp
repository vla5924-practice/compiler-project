#pragma once

#include "parser/handlers/base_handler.hpp"

namespace parser {

class FunctionDefinitionHandler : public BaseHandler {
    bool functionArgumentsEnd;
    bool functionBegin;

  public:
    virtual void run(ParserState &state) override;
    virtual void reset() override;
};

} // namespace parser
