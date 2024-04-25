#include "semantizer/semantizer.hpp"

#include "compiler/optree/operation.hpp"
#include "compiler/optree/program.hpp"

#include "semantizer/semantizer_context.hpp"

using namespace optree;
using namespace optree::semantizer;

void Semantizer::process(const Program &program) {
    return process(program.root);
}

void Semantizer::process(const Operation::Ptr &op) {
    SemantizerContext ctx;

    if (!ctx.errors.empty())
        throw ctx.errors;
}
