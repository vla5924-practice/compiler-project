#pragma once

#include "parser/handlers/base_handler.hpp"

namespace parser {

class VariableDeclarationHandler : public BaseHandler {
    bool wasInDefinition;

  public:
    virtual void run(ParserState &state) override;
    virtual void reset() override;
};

} // namespace parser
