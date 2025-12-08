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

#include "demo.h"

#include "error.h"
#include "gql/ast/algorithm.h"
#include "helpers/check_label_expression.h"

namespace gql_demo {

Value Demo::Evaluate(const gql::ast::PropertyExistsPredicate& expr,
                     const ExecutionContext& context) {
  auto value = Evaluate(expr.element, context);
  return gql::ast::variant_switch(
      value.v, [&](const auto&) -> Value { ThrowInvalidValueTypeError(expr); },
      [&](Null) { return Value{Null{}}; },
      [&](const NodeReference& value) {
        if (auto* element = graph.GetNode(value.nodeId)) {
          return Value{element->properties.count(expr.property.name) != 0};
        }
        ThrowInvalidReferenceError(expr.element);
      },
      [&](const EdgeReference& value) {
        if (auto* element = graph.GetEdge(value.edgeId)) {
          return Value{element->properties.count(expr.property.name) != 0};
        }
        ThrowInvalidReferenceError(expr.element);
      });
}

Value Demo::Evaluate(const gql::ast::DirectedPredicate& expr,
                     const ExecutionContext& context) {
  auto value = Evaluate(expr.element, context);
  return gql::ast::variant_switch(
      value.v, [&](const auto&) -> Value { ThrowInvalidValueTypeError(expr); },
      [&](Null) { return Value{Null{}}; },
      [&](const EdgeReference& value) {
        if (auto* edge = graph.GetEdge(value.edgeId)) {
          return Value{edge->directed != expr.isNot};
        }
        ThrowInvalidReferenceError(expr.element);
      });
}

Value Demo::Evaluate(const gql::ast::LabeledPredicate& expr,
                     const ExecutionContext& context) {
  auto value = Evaluate(expr.element, context);
  return gql::ast::variant_switch(
      value.v, [&](const auto&) -> Value { ThrowInvalidValueTypeError(expr); },
      [&](Null) { return Value{Null{}}; },
      [&](const NodeReference& value) {
        if (auto* element = graph.GetNode(value.nodeId)) {
          bool result =
              helpers::CheckLabelExpression(expr.label, graph, element->labels);
          return Value{result != expr.isNot};
        }
        ThrowInvalidReferenceError(expr.element);
      },
      [&](const EdgeReference& value) {
        if (auto* element = graph.GetEdge(value.edgeId)) {
          bool result =
              helpers::CheckLabelExpression(expr.label, graph, element->labels);
          return Value{result != expr.isNot};
        }
        ThrowInvalidReferenceError(expr.element);
      });
}

Value Demo::Evaluate(const gql::ast::SourceDestinationPredicate& expr,
                     const ExecutionContext& context) {
  auto nodeValue = Evaluate(expr.node, context);
  auto edgeValue = Evaluate(expr.edge, context);
  if (std::holds_alternative<Null>(nodeValue.v)) {
    return Null{};
  }
  auto* nodeRef = std::get_if<NodeReference>(&nodeValue.v);
  if (!nodeRef) {
    ThrowInvalidValueTypeError(expr.node);
  }
  return gql::ast::variant_switch(
      edgeValue.v,
      [&](const auto&) -> Value { ThrowInvalidValueTypeError(expr.edge); },
      [&](Null) { return Value{Null{}}; },
      [&](const EdgeReference& value) -> Value {
        if (auto* edge = graph.GetEdge(value.edgeId)) {
          if (!edge->directed) {
            return false;
          }
          switch (expr.kind) {
            case gql::ast::SourceDestinationPredicate::Kind::
                NodeIsDestinationOfEdge:
              return (edge->target == nodeRef->nodeId) != expr.isNot;
            case gql::ast::SourceDestinationPredicate::Kind::NodeIsSourceOfEdge:
              return (edge->source == nodeRef->nodeId) != expr.isNot;
          }
        }
        ThrowInvalidReferenceError(expr.edge);
      });
}

Value Demo::Evaluate(const gql::ast::AllElementsPredicate& expr,
                     const ExecutionContext& context) {
  std::unordered_set<Graph::EdgeOrNodeId> elementIds;
  std::optional<bool> isNodes;
  for (auto& el : expr.elements) {
    auto value = Evaluate(el, context);
    bool checkFailed = false;
    if (!gql::ast::variant_switch(
            value.v,
            [&](const auto&) -> bool { ThrowInvalidValueTypeError(el); },
            [&](Null) -> bool {
              throw GQLRuntimeError(el.inputPosition(),
                                    "data exception - null value not allowed");
            },
            [&](const NodeReference& value) {
              if (isNodes.has_value() && !isNodes.value()) {
                // data exception � values not comparable (22G04)
                throw GQLRuntimeError(el.inputPosition(),
                                      "data exception - values not comparable");
              }
              isNodes = true;

              switch (expr.kind) {
                case gql::ast::AllElementsPredicate::Kind::AllDifferent:
                  if (!elementIds.insert(value.nodeId).second) {
                    return false;
                  }
                  break;
                case gql::ast::AllElementsPredicate::Kind::Same:
                  if (elementIds.empty()) {
                    elementIds.insert(value.nodeId);
                  } else if (*elementIds.begin() != value.nodeId) {
                    return false;
                  }
                  break;
              }
              return true;
            },
            [&](const EdgeReference& value) {
              if (isNodes.has_value() && isNodes.value()) {
                // data exception � values not comparable (22G04)
                throw GQLRuntimeError(el.inputPosition(),
                                      "data exception - values not comparable");
              }
              isNodes = false;

              switch (expr.kind) {
                case gql::ast::AllElementsPredicate::Kind::AllDifferent:
                  if (!elementIds.insert(value.edgeId).second) {
                    checkFailed = true;
                  }
                  break;
                case gql::ast::AllElementsPredicate::Kind::Same:
                  if (elementIds.empty()) {
                    elementIds.insert(value.edgeId);
                  } else if (*elementIds.begin() != value.edgeId) {
                    return false;
                  }
                  break;
              }
              return true;
            })) {
      return false;
    }
  }
  return true;
}

}  // namespace gql_demo