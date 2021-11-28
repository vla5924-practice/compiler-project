#pragma once

#include "parser/handlers/base_handler.hpp"

namespace parser {

class ElifStatementHandler : public BaseHandler {
    enum class Branch {
        None,
        Elif,
        Else,
    };
    Branch branch;
    bool wasInExpression;

  public:
    virtual void run(ParserState &state) override;
    virtual void reset() override;
};

} // namespace parser
