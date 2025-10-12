#pragma once

#include "gql/ast/algorithm.h"
#include "gql/ast/nodes/node.h"

namespace gql::rewrite {

namespace detail {

struct SetInputPositionVisitor {
  explicit SetInputPositionVisitor(const ast::InputPosition& position)
      : position(position) {}

  template <
      typename NodeType,
      typename = std::enable_if_t<!std::is_base_of_v<ast::Node, NodeType>>>
  auto operator()(NodeType*) const {
    return ast::VisitorResult::kContinue;
  }

  auto operator()(ast::Node* node) const {
    if (node->inputPosition().IsSet()) {
      return ast::VisitorResult::kSkipChildren;
    }
    node->SetInputPosition(position);
    return ast::VisitorResult::kContinue;
  }

 private:
  const ast::InputPosition position;
};

}  // namespace detail

template <typename NodeType>
inline void SetInputPositionRecursive(NodeType& node,
                                      const ast::InputPosition& position) {
  ast::ForEachNodeInTree(node, detail::SetInputPositionVisitor(position));
}

}  // namespace gql::rewrite