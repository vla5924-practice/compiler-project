#include "semantizer/semantizer.hpp"

#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/operation.hpp"
#include "compiler/optree/program.hpp"

#include "semantizer/semantizer_context.hpp"

#define VERIFY(ADAPTOR_CLASS_NAME)                                                                                     \
    template <>                                                                                                        \
    void verify<ADAPTOR_CLASS_NAME>(const Operation::Ptr &op, SemantizerContext &ctx)

using namespace optree;
using namespace optree::semantizer;

template <typename AdaptorType>
void verify(const Operation::Ptr &op, SemantizerContext &ctx) {
    std::terminate();
}

VERIFY(ModuleOp) {
}

void verify(const Operation::Ptr &op, SemantizerContext &ctx) {
    if (op->is<ModuleOp>())
        return verify<ModuleOp>(op, ctx);
}

void Semantizer::process(const Program &program) {
    process(program.root);
}

void Semantizer::process(const Operation::Ptr &op) {
    SemantizerContext ctx;

    if (!ctx.errors.empty())
        throw ctx.errors;
}
