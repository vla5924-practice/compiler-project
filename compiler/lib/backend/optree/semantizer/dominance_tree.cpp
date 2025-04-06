#include "semantizer/dominance_tree.hpp"

#include "compiler/optree/adaptors.hpp"
#include "compiler/optree/operation.hpp"
#include "compiler/utils/helpers.hpp"

using namespace optree;
using namespace optree::semantizer;

DominanceTree::DominanceTree(const Operation::Ptr &rootOp) : root(nullptr) {
    root = new Node(nullptr);
    traversedOps[rootOp.get()] = root;
    traverseOp(root, rootOp);
}

DominanceTree::~DominanceTree() {
    if (root == nullptr)
        return;
    for (auto it : traversedOps)
        delete it.second;
    root = nullptr;
    traversedOps.clear();
}

bool DominanceTree::dominates(const Operation::Ptr &dominator, const Operation::Ptr &dominated) const {
    return dominator == dominated || properlyDominates(dominator, dominated);
}

bool DominanceTree::properlyDominates(const Operation::Ptr &dominator, const Operation::Ptr &dominated) const {
    const Node *dtorNode = findNode(dominator);
    const Node *dtedNode = findNode(dominated);
    if (dtorNode == nullptr || dtedNode == nullptr)
        return false;
    const Node *current = dtedNode->parent;
    while (current) {
        if (current == dtorNode)
            return true;
        current = current->parent;
    }
    return false;
}

void DominanceTree::dump(std::ostream &str) const {
    str << "DominanceTree {\n";
    for (const auto &[op, node] : traversedOps)
        str << "  " << op->name << " [" << op << "] -> node [" << node << "] with parent [" << node->parent << "]\n";
    str << "}\n";
}

void DominanceTree::traverseOp(Node *parent, const Operation::Ptr &op) {
    bool isSSAOp = true;
    if (utils::isAny<ModuleOp, IfOp>(op))
        isSSAOp = false;
    traverseOpImpl(parent, op, isSSAOp);
}

void DominanceTree::traverseOpImpl(Node *parent, const Operation::Ptr &op, bool isSSAOp) {
    for (const auto &childOp : op->body) {
        auto *node = new Node(parent);
        traversedOps[childOp.get()] = node;
        traverseOp(node, childOp);
        if (isSSAOp && !childOp->is<ConditionOp>())
            parent = node;
    }
}

const DominanceTree::Node *DominanceTree::findNode(const Operation::Ptr &op) const {
    auto it = traversedOps.find(op.get());
    if (it != traversedOps.end())
        return it->second;
    return nullptr;
}
