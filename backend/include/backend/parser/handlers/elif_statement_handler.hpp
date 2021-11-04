#pragma once

#include "parser/handlers/base_handler.hpp"

namespace parser {

class ElifStatementHandler : public BaseHandler {
    bool wasInExpression;

  public:
    virtual void run(ParserState &state) override;
    virtual void reset() override;
};

} // namespace parser
