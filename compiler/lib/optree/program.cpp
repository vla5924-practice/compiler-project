#include "program.hpp"

#include "adaptors.hpp"
#include "operation.hpp"

using namespace optree;

Program::~Program() {
    if (root)
        root->erase();
    // Create any operation to force storage cleanup
    root = Operation::make<ModuleOp>().op;
}
