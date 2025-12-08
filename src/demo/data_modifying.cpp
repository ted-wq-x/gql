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

namespace gql_demo {

OptBindingTable Demo::Process(
    const gql::ast::LinearDataModifyingStatementBody& statement,
    ExecutionContext& context) {
  for (auto& stmt : statement.statements) {
    gql::ast::variant_switch(
        stmt,
        [&](const gql::ast::SimpleQueryStatement& query) {
          Process(query, context);
        },
        [&](const gql::ast::PrimitiveDataModifyingStatement& prim) {
          Process(prim, context);
        },
        [&](const gql::ast::CallProcedureStatement& callProc) {
          Process(callProc, context);
        });
  }
  if (statement.result) {
    return ProcessAndCheckType(*statement.result, context);
  }
  return {};
}

Graph::Element* Demo::FindElementByVariable(
    const gql::ast::BindingVariableReference& var,
    const ExecutionContext& context) {
  auto& value = context.workingRecord.fieldValue(var.name);
  return gql::ast::variant_switch(
      value.v,
      [&](const auto&) -> Graph::Element* { ThrowInvalidValueTypeError(var); },
      [&](const Null&) -> Graph::Element* { return nullptr; },
      [&](const NodeReference& nodeRef) -> Graph::Element* {
        if (auto* node = graph.GetNode(nodeRef.nodeId)) {
          return node;
        }
        ThrowInvalidReferenceError(var);
      },
      [&](const EdgeReference& edgeRef) -> Graph::Element* {
        if (auto* edge = graph.GetEdge(edgeRef.edgeId)) {
          return edge;
        }
        ThrowInvalidReferenceError(var);
      });
}

void Demo::Process(const gql::ast::PrimitiveDataModifyingStatement& value,
                   ExecutionContext& context) {
  // Execute primitive data modifying statement
  gql::ast::variant_switch(
      value,
      [&](const gql::ast::InsertStatement& value) { Process(value, context); },
      [&](const gql::ast::SetStatement& value) {
        ContextStateChecker contextStateChecker(value, context);

        for (size_t recordIdx = 0; recordIdx < context.workingTable.size();
             recordIdx++) {
          auto childContext = context.MakeAmended(recordIdx);
          for (const auto& item : value.items) {
            gql::ast::variant_switch(
                item,
                [&](const gql::ast::SetPropertyItem& value) {
                  if (auto* el =
                          FindElementByVariable(value.var, childContext)) {
                    el->properties[value.property.name] =
                        Evaluate(value.value, childContext);
                  }
                },
                [&](const gql::ast::SetAllPropertiesItem& value) {
                  if (auto* el =
                          FindElementByVariable(value.var, childContext)) {
                    el->properties.clear();
                    for (auto& prop : value.properties) {
                      el->properties[prop.name.name] =
                          Evaluate(prop.value, childContext);
                    }
                  }
                },
                [&](const gql::ast::SetLabelItem& value) {
                  if (auto* el =
                          FindElementByVariable(value.var, childContext)) {
                    el->labels.insert(value.label.name);
                  }
                });
          }
        }
      },
      [&](const gql::ast::RemoveStatement& value) {
        ContextStateChecker contextStateChecker(value, context);

        for (size_t recordIdx = 0; recordIdx < context.workingTable.size();
             recordIdx++) {
          auto childContext = context.MakeAmended(recordIdx);
          for (const auto& item : value.items) {
            gql::ast::variant_switch(
                item,
                [&](const gql::ast::RemovePropertyItem& value) {
                  if (auto* el =
                          FindElementByVariable(value.var, childContext)) {
                    el->properties.erase(value.property.name);
                  }
                },
                [&](const gql::ast::RemoveLabelItem& value) {
                  if (auto* el =
                          FindElementByVariable(value.var, childContext)) {
                    el->labels.erase(value.label.name);
                  }
                });
          }
        }
      },
      [&](const gql::ast::DeleteStatement& value) { Process(value, context); });
}

void Demo::Process(const gql::ast::InsertStatement& value,
                   ExecutionContext& context) {
  ContextStateChecker contextStateChecker(value, context);

  BindingTable newTable;
  auto inserter = newTable.CreateColumnInserter();
  inserter.Reserve(context.workingTable.size());
  for (size_t recordIdx = 0; recordIdx < context.workingTable.size();
       recordIdx++) {
    auto childContext = context.MakeAmended(recordIdx);
    std::unordered_map<std::string, Graph::NodeId> createdNodes;
    for (auto& path : value.paths) {
      Graph::Node* lastNode = nullptr;
      auto edgeIt = path.edges.begin();
      for (auto& nodePattern : path.nodes) {
        Graph::Node* node;
        if (nodePattern.filler.var) {
          if (auto* boundNode = childContext.workingRecord.hasField(
                  nodePattern.filler.var->name)) {
            auto nodeId = std::get<NodeReference>(boundNode->v).nodeId;
            node = graph.GetNode(nodeId);
            if (!node) {
              ThrowInvalidReferenceError(nodePattern);
            }
          } else {
            auto [outNodeIt, inserted] =
                createdNodes.emplace(nodePattern.filler.var->name, 0);
            if (inserted) {
              node = &graph.AddNode();
              outNodeIt->second = node->id;
              inserter.Set(nodePattern.filler.var->name,
                           NodeReference{node->id});
            } else {
              node = graph.GetNode(outNodeIt->second);
            }
          }
        } else {
          node = &graph.AddNode();
        }
        for (auto& label : nodePattern.filler.labels.labels) {
          node->labels.insert(label.name);
        }
        for (auto& prop : nodePattern.filler.props) {
          node->properties[prop.name.name] = Evaluate(prop.value, childContext);
        }
        if (lastNode) {
          auto edgeNode1 = lastNode;
          auto edgeNode2 = node;
          if (edgeIt->direction ==
              gql::ast::InsertEdgePattern::Direction::Left) {
            std::swap(edgeNode1, edgeNode2);
          }
          auto& edge = graph.AddEdge(
              edgeNode1, edgeNode2,
              edgeIt->direction !=
                  gql::ast::InsertEdgePattern::Direction::Undirected);

          for (const auto& label : edgeIt->filler.labels.labels) {
            edge.labels.insert(label());
          }
          for (auto& prop : edgeIt->filler.props) {
            edge.properties[prop.name.name] =
                Evaluate(prop.value, childContext);
          }

          if (edgeIt->filler.var) {
            inserter.Set(edgeIt->filler.var->name, EdgeReference{edge.id});
          }

          edgeIt++;
        }
        lastNode = node;
      }
    }
    inserter.NextRow();
  }
  context.workingTable.Concat(std::move(newTable));
}

void Demo::Process(const gql::ast::DeleteStatement& statement,
                   ExecutionContext& context) {
  ContextStateChecker contextStateChecker(statement, context);
  for (size_t recordIdx = 0; recordIdx < context.workingTable.size();
       recordIdx++) {
    auto childContext = context.MakeAmended(recordIdx);
    for (auto& item : statement.items) {
      auto element = Evaluate(item, childContext);
      std::unordered_set<Graph::NodeId> nodes;
      std::unordered_set<Graph::NodeId> edges;
      gql::ast::variant_switch(
          element.v, [&](const auto&) { ThrowInvalidValueTypeError(item); },
          [](const Null&) {},
          [&](const NodeReference& nodeRef) { nodes.insert(nodeRef.nodeId); },
          [&](const EdgeReference& edgeRef) { edges.insert(edgeRef.edgeId); });
      for (auto nodeId : nodes) {
        if (auto* node = graph.GetNode(nodeId)) {
          for (auto edgeId : node->edges) {
            if (edges.insert(edgeId).second && !statement.detach) {
              throw GQLRuntimeError(
                  item.inputPosition(),
                  "dependent object error - edges still exist");
            }
          }
        }
      }
      for (auto edgeId : edges) {
        if (!graph.DeleteEdge(edgeId)) {
          ThrowInvalidReferenceError(item);
        }
      }
      for (auto nodeId : nodes) {
        if (!graph.DeleteNode(nodeId)) {
          ThrowInvalidReferenceError(item);
        }
      }
    }
  }
}

}  // namespace gql_demo