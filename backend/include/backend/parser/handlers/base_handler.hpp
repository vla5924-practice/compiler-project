#pragma once

#include "parser/parser_state.hpp"

namespace parser {

class BaseHandler {
  public:
    BaseHandler();
    virtual ~BaseHandler() = default;

    virtual void run(ParserState &state) = 0;
    virtual void reset();
};

} // namespace parser
