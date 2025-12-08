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

#include "operators.h"

#include "binding_table.h"
#include "gql/ast/nodes/expression.h"
#include "helpers/check_label_expression.h"
#include "helpers/scope_exit.h"

namespace gql_demo {

void SearchOp::SetNextOp(SearchOpPtr op) {
  assert(!nextOp);
  nextOp = std::move(op);
}

void ScanAllNodesOp::Execute(SearchContext& context) {
  for (auto& node : context.graph().nodes()) {
    context.currentPath().PushNode(node.id);
    nextOp->Execute(context);
    context.currentPath().PopNode();
  }
}

void QuantifiedOp::Execute(SearchContext& context) {
  if (!upperCount || counter < *upperCount) {
    counter++;
    loopOp->Execute(context);
    counter--;
  }
  if (counter >= lowerCount) {
    nextOp->Execute(context);
  }
}

ExpandEdgeOp::ExpandEdgeOp(gql::ast::EdgeDirectionPattern direction)
    : direction(direction) {}

void ExpandEdgeOp::JoinEdge(const std::string& var) {
  joinVar = var;
}

void ExpandEdgeOp::SetLabelExpression(const gql::ast::LabelExpression& expr) {
  labelExpression = expr;
}

void ExpandEdgeOp::Execute(SearchContext& context) {
  std::optional<Graph::EdgeId> joinEdge;
  if (!joinVar.empty()) {
    auto& varBindings = context.boundElementVariable(joinVar);
    assert(!varBindings.empty());
    if (context.currentPath().reverseDirection()) {
      joinEdge = varBindings.front();
    } else {
      joinEdge = varBindings.back();
    }
  }

  if (context.currentPath().empty()) {
    // Edge is the first element in the path.
    assert(joinEdge);
    auto& edge = *context.graph().GetEdge(*joinEdge);
    if (CheckEdge(context, edge, edge.source)) {
      context.currentPath().PushNode(edge.source);
      if (context.currentPath().PushEdgeNode(*joinEdge, edge.target)) {
        nextOp->Execute(context);
        context.currentPath().PopEdgeNode();
      }
      context.currentPath().PopNode();
    }
    if (CheckEdge(context, edge, edge.target)) {
      context.currentPath().PushNode(edge.target);
      if (context.currentPath().PushEdgeNode(*joinEdge, edge.source)) {
        nextOp->Execute(context);
        context.currentPath().PopEdgeNode();
      }
      context.currentPath().PopNode();
    }
  } else {
    auto currentNodeId = context.currentPath().currentNode();
    for (auto edgeId : context.graph().GetNode(currentNodeId)->edges) {
      if (joinEdge && edgeId != *joinEdge) {
        continue;
      }

      auto& edge = *context.graph().GetEdge(edgeId);

      if (!CheckEdge(context, edge, currentNodeId)) {
        continue;
      }

      if (!context.currentPath().PushEdgeNode(
              edgeId,
              edge.source == currentNodeId ? edge.target : edge.source)) {
        continue;
      }

      nextOp->Execute(context);

      context.currentPath().PopEdgeNode();
    }
  }
}

bool ExpandEdgeOp::CheckEdge(SearchContext& context,
                             const Graph::Edge& edge,
                             Graph::NodeId currentNodeId) const {
  if (labelExpression && !helpers::CheckLabelExpression(
                             *labelExpression, context.graph(), edge.labels)) {
    return false;
  }

  std::optional<bool> rightDirection;
  if (edge.directed) {
    rightDirection = edge.source == currentNodeId;
    if (context.currentPath().reverseDirection()) {
      rightDirection = !*rightDirection;
    }
  }

  if (!CheckDirection(rightDirection)) {
    if (edge.directed && edge.source == edge.target) {
      // Check for self-loop.
      return CheckDirection(!*rightDirection);
    }
    return false;
  }
  return true;
}

bool ExpandEdgeOp::CheckDirection(std::optional<bool> rightDirection) const {
  switch (direction) {
    case gql::ast::EdgeDirectionPattern::Left:
      return rightDirection && !*rightDirection;
    case gql::ast::EdgeDirectionPattern::Undirected:
      return !rightDirection;
    case gql::ast::EdgeDirectionPattern::Right:
      return rightDirection && *rightDirection;
    case gql::ast::EdgeDirectionPattern::LeftOrUndirected:
      return !rightDirection || !*rightDirection;
    case gql::ast::EdgeDirectionPattern::UndirectedOrRight:
      return !rightDirection || *rightDirection;
    case gql::ast::EdgeDirectionPattern::LeftOrRight:
      return !!rightDirection;
    case gql::ast::EdgeDirectionPattern::AnyDirection:
      return true;
    default:
      assert(false);
      return false;
  }
}

void JoinNodeOp::Execute(SearchContext& context) {
  auto& varBindings = context.boundElementVariable(var);
  assert(!varBindings.empty());
  Graph::NodeId nodeId;
  if (context.currentPath().reverseDirection()) {
    nodeId = varBindings.front();
  } else {
    nodeId = varBindings.back();
  }

  if (context.currentPath().empty()) {
    assert(varBindings.size() == 1);

    context.currentPath().PushNode(nodeId);
    nextOp->Execute(context);
    context.currentPath().PopNode();
  } else {
    if (context.currentPath().currentNode() != nodeId) {
      return;
    }
    nextOp->Execute(context);
  }
}

void ReferenceCurrentNodeOp::Execute(SearchContext& context) {
  if (context.currentPath().ReferenceCurrentNode()) {
    nextOp->Execute(context);
    context.currentPath().UnreferenceCurrentNode();
  }
}

BindElementOp::BindElementOp(const std::string& var, bool isNode)
    : var(var), isNode(isNode) {}

void BindElementOp::Execute(SearchContext& context) {
  Graph::EdgeOrNodeId elementId = isNode ? context.currentPath().currentNode()
                                         : context.currentPath().currentEdge();

  auto& varBindings = context.boundElementVariable(var);
  if (context.currentPath().reverseDirection()) {
    varBindings.push_front(elementId);
    nextOp->Execute(context);
    varBindings.pop_front();
  } else {
    varBindings.push_back(elementId);
    nextOp->Execute(context);
    varBindings.pop_back();
  }
}

AddConditionClosureOp::AddConditionClosureOp(size_t closureIdx)
    : closureIdx(closureIdx) {}

void AddConditionClosureOp::Execute(SearchContext& context) {
  if (context.searchConditionClosures.size() <= closureIdx) {
    // TODO: Make more appropriate fix
    context.searchConditionClosures.resize(closureIdx + 1);
  }
  auto& closure = context.searchConditionClosures[closureIdx].emplace_back();
  for (auto& var : variables) {
    closure.variableBindings[var] = context.currentBindings[var].reference();
  }

  nextOp->Execute(context);

  context.searchConditionClosures[closureIdx].pop_back();
}

std::shared_ptr<SearchOp> VariableBindingsLevelOp::CreateExitOp() {
  auto exitOp = std::make_shared<VariableBindingsLevelOp>();
  exitOp->variables = variables;
  exitOp->startOp = false;
  return exitOp;
}

void VariableBindingsLevelOp::Execute(SearchContext& context) {
  if (startOp) {
    for (auto& var : variables) {
      context.currentBindings[var].PushLevel();
    }

    nextOp->Execute(context);

    for (auto& var : variables) {
      context.currentBindings[var].PopLevel();
    }
  } else {
    for (auto& var : variables) {
      context.currentBindings[var].PopLevel();
    }

    nextOp->Execute(context);

    for (auto& var : variables) {
      context.currentBindings[var].PushLevel();
    }
  }
}

std::shared_ptr<VariableBindingsOp> VariableBindingsOp::CreateExitOp() {
  assert(!sharedState);
  sharedState = std::make_shared<SharedState>();
  auto exitOp = std::make_shared<VariableBindingsOp>();
  exitOp->variables = variables;
  exitOp->startOp = false;
  exitOp->sharedState = sharedState;
  return exitOp;
}

void VariableBindingsOp::Execute(SearchContext& context) {
  auto& bindingsStartSize = sharedState->bindingsStartSize;
  if (startOp) {
    bindingsStartSize.resize(variables.size());
    for (size_t i = 0; i < variables.size(); i++) {
      auto& var = variables[i].first;
      bindingsStartSize[i] = context.boundElementVariable(var).size();
    }

    nextOp->Execute(context);
  } else {
    std::vector<bool> needsBindingsReset(variables.size());
    for (size_t i = 0; i < variables.size(); i++) {
      auto& [var, isSingleton] = variables[i];
      auto& contextVar = context.currentBindings[var];
      auto& boundElements = context.boundElementVariable(var);
      SearchContext::ElementBindingsRange bindings;
      bindings.second = boundElements.size() - bindingsStartSize[i];
      if (context.currentPath().reverseDirection()) {
        bindings.first = boundElements.begin();
      } else {
        bindings.first = std::next(boundElements.begin(), bindingsStartSize[i]);
      }
      needsBindingsReset[i] = contextVar.Bind(bindings, isSingleton);
    }
    auto oldValue = std::move(bindingsStartSize);

    nextOp->Execute(context);

    bindingsStartSize.swap(oldValue);
    for (size_t i = 0; i < variables.size(); i++) {
      auto& contextVar = context.currentBindings[variables[i].first];
      if (needsBindingsReset[i]) {
        contextVar.ResetBindings();
      }
    }
  }
}

CheckConditionOp::CheckConditionOp(
    const gql::ast::ValueExpression& cond,
    const gql::syntax_analyzer::GraphPatternWhereClauseAuxData& auxData,
    size_t closureIdx)
    : condition(cond), conditionAuxData(auxData), closureIdx(closureIdx) {}

void CheckConditionOp::Execute(SearchContext& context) {
  auto closures = std::move(context.searchConditionClosures[closureIdx]);
  helpers::ScopeExit scopeExit;
  scopeExit.Add(
      [&]() { context.searchConditionClosures[closureIdx].swap(closures); });
  for (auto& closure : closures) {
    BindingTable graphVarsTable;
    for (auto& [var, _] : conditionAuxData.referencedVariables) {
      graphVarsTable.AddColumn(var);
    }
    auto inserter = graphVarsTable.CreateInserter();
    for (auto& [var, varData] : conditionAuxData.referencedVariables) {
      switch (varData.type) {
        case gql::GraphPatternVariableType::Node:
        case gql::GraphPatternVariableType::Edge: {
          auto bindingsIt = closure.variableBindings.find(var);
          assert(bindingsIt != closure.variableBindings.end());
          assert(bindingsIt->second->bindings.has_value());
          inserter.Set(var, context.GetElementBindingsValue(
                                varData.type, varData.degreeOfExposure,
                                *bindingsIt->second->bindings));
          break;
        }
        case gql::GraphPatternVariableType::Path:
          inserter.Set(var, context.pathReferenceValue(var));
          break;
        default:
          assert(false);
          break;
      }
    }
    WorkingRecord expressionRecord;
    expressionRecord.parentRecord = &context.workingRecord;
    expressionRecord.table = &graphVarsTable;
    expressionRecord.index = 0;
    if (!context.evaluateSearchCondition(condition, expressionRecord)) {
      return;
    }
  }
  nextOp->Execute(context);
}

void SetCurrentPathOp::Execute(SearchContext& context) {
  const auto oldValue = context.currentPathIndex;
  context.currentPathIndex = pathIndex;
  context.currentPath().AssertEmpty();
  nextOp->Execute(context);
  context.currentPathIndex = oldValue;
}

void SetCurrentPathReverseDirectionOp::Execute(SearchContext& context) {
  const auto oldValue = context.currentPath().reverseDirection();
  context.currentPath().SetReverseDirection(reverse);
  nextOp->Execute(context);
  context.currentPath().SetReverseDirection(oldValue);
}

void CheckNodeLabelsOp::Execute(SearchContext& context) {
  auto nodeId = context.currentPath().currentNode();
  auto& node = *context.graph().GetNode(nodeId);
  if (!helpers::CheckLabelExpression(labelExpression, context.graph(),
                                     node.labels)) {
    return;
  }

  nextOp->Execute(context);
}

void PathModeUniqueEdgesOp::Execute(SearchContext& context) {
  if (startOp) {
    assert(!context.currentPath().matchedEdges);
    context.currentPath().matchedEdges.emplace();
    if (restoreStateOnEnter) {
      context.currentPath().matchedEdges->swap(
          context.currentPath().savedMatchedEdges);
    }

    nextOp->Execute(context);

    if (restoreStateOnEnter) {
      context.currentPath().matchedEdges->swap(
          context.currentPath().savedMatchedEdges);
    }
    context.currentPath().matchedEdges.reset();
  } else {
    assert(context.currentPath().matchedEdges);
    auto oldValue = std::move(*context.currentPath().matchedEdges);
    if (saveStateOnExit) {
      oldValue.swap(context.currentPath().savedMatchedEdges);
    }
    context.currentPath().matchedEdges.reset();

    nextOp->Execute(context);

    if (saveStateOnExit) {
      oldValue.swap(context.currentPath().savedMatchedEdges);
    }
    context.currentPath().matchedEdges = std::move(oldValue);
  }
}

void PathModeUniqueNodesOp::Execute(SearchContext& context) {
  if (startOp) {
    assert(!context.currentPath().matchedNodes);
    context.currentPath().matchedNodes.emplace();
    if (restoreStateOnEnter) {
      context.currentPath().matchedNodes->swap(
          context.currentPath().savedMatchedNodes);
    }

    nextOp->Execute(context);

    if (restoreStateOnEnter) {
      context.currentPath().matchedNodes->swap(
          context.currentPath().savedMatchedNodes);
    }
    context.currentPath().matchedNodes.reset();
  } else {
    assert(context.currentPath().matchedNodes);
    SearchedPath::MatchedNodes oldValue;
    // There are iterators to the elements in the matchedNodes, swap gives more
    // guarantees than move assignment about keeping them valid.
    oldValue.swap(*context.currentPath().matchedNodes);
    if (saveStateOnExit) {
      oldValue.swap(context.currentPath().savedMatchedNodes);
    }
    context.currentPath().matchedNodes.reset();

    nextOp->Execute(context);

    if (saveStateOnExit) {
      oldValue.swap(context.currentPath().savedMatchedNodes);
    }
    assert(!context.currentPath().matchedNodes.has_value());
    context.currentPath().matchedNodes.emplace().swap(oldValue);
  }
}

}  // namespace gql_demo