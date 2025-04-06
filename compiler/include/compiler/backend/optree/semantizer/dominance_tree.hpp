#pragma once

#include <ostream>
#include <unordered_map>

#include "compiler/optree/operation.hpp"

namespace optree {
namespace semantizer {

struct DominanceTree {
    explicit DominanceTree(const Operation::Ptr &rootOp);

    DominanceTree() = delete;
    DominanceTree(const DominanceTree &) = delete;
    DominanceTree(DominanceTree &&) = default;
    ~DominanceTree();

    bool dominates(const Operation::Ptr &dominator, const Operation::Ptr &dominated) const;
    bool properlyDominates(const Operation::Ptr &dominator, const Operation::Ptr &dominated) const;

    void dump(std::ostream &str) const;

  private:
    struct Node {
        Node *parent = nullptr;
    };

    void traverseOp(Node *parent, const Operation::Ptr &op);
    void traverseOpImpl(Node *parent, const Operation::Ptr &op, bool isSSAOp);
    const Node *findNode(const Operation::Ptr &op) const;

    Node *root;
    std::unordered_map<const Operation *, const Node *> traversedOps;
};

} // namespace semantizer
} // namespace optree
