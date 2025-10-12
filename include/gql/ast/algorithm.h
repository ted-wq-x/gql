// Copyright 2025 Oleg Maximenko
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "gql/ast/detail/enumerate_children.h"

namespace gql::ast {

template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;

  overloaded(Ts&&... ts) : Ts{std::forward<Ts>(ts)}... {}
};

// Deduction guide
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

template <class Variant, class... Ts>
auto variant_switch(Variant&& variant, Ts&&... ts) {
  return std::visit(overloaded<Ts...>(std::forward<Ts>(ts)...),
                    std::forward<Variant>(variant));
}

enum class VisitorResult {
  kContinue,
  kSkipChildren,
  kStop,
};

template <typename NodeType, typename VisitorType>
bool ForEachNodeInTree(NodeType& node, VisitorType&& visitor) {
  switch (visitor(&node)) {
    case VisitorResult::kContinue:
      break;
    case VisitorResult::kSkipChildren:
      return true;
    case VisitorResult::kStop:
      return false;
  }
  return EnumerateChildren<NodeType>::ForEachChild(
      node,
      [&visitor](auto& node) { return ForEachNodeInTree(node, visitor); });
}

template <typename NodeType, typename VisitorType>
bool ForEachDescendantNodeInTree(NodeType& node, VisitorType&& visitor) {
  return EnumerateChildren<NodeType>::ForEachChild(
      node,
      [&visitor](auto& node) { return ForEachNodeInTree(node, visitor); });
}

template <typename NodeType, typename VisitorType>
bool ForEachChild(NodeType& node, VisitorType&& visitor) {
  return EnumerateChildren<NodeType>::ForEachChild(
      node, std::forward<VisitorType>(visitor));
}

namespace detail {

template <typename TargetType>
struct FindFirstNodeOfTypeVisitor {
  template <typename NodeType>
  auto operator()(NodeType*) {
    return VisitorResult::kContinue;
  }

  auto operator()(TargetType* node) {
    result = node;
    return VisitorResult::kStop;
  }

  TargetType* result = nullptr;
};

template <typename TargetType, typename Func>
struct ForEachNodeOfTypeVisitor {
  explicit ForEachNodeOfTypeVisitor(Func& func) : func(func) {}

  template <typename NodeType>
  auto operator()(NodeType*) const {
    return VisitorResult::kContinue;
  }

  VisitorResult operator()(TargetType* node) const { return func(*node); }

 private:
  Func& func;
};

}  // namespace detail

template <typename NodeType, typename RootType>
NodeType* FindFirstNodeOfType(RootType& root) {
  detail::FindFirstNodeOfTypeVisitor<NodeType> visitor;
  ForEachNodeInTree(root, visitor);
  return visitor.result;
}

template <typename NodeType, typename RootType>
const NodeType* FindFirstNodeOfType(const RootType& root) {
  detail::FindFirstNodeOfTypeVisitor<const NodeType> visitor;
  ForEachNodeInTree(root, visitor);
  return visitor.result;
}

template <typename NodeType, typename RootType>
const NodeType* FindFirstDescendantNodeOfType(const RootType& root) {
  detail::FindFirstNodeOfTypeVisitor<const NodeType> visitor;
  ForEachDescendantNodeInTree(root, visitor);
  return visitor.result;
}

template <typename NodeType, typename RootType, typename Func>
void ForEachNodeOfType(RootType& root, Func&& func) {
  detail::ForEachNodeOfTypeVisitor<NodeType, Func> visitor(func);
  ForEachNodeInTree(root, visitor);
}

template <typename NodeType, typename RootType, typename Func>
void ForEachNodeOfType(const RootType& root, Func&& func) {
  detail::ForEachNodeOfTypeVisitor<const NodeType, Func> visitor(func);
  ForEachNodeInTree(root, visitor);
}

}  // namespace gql::ast