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

#include "search_context.h"

#include "helpers/scope_exit.h"
#include "timeout_watchdog.h"

namespace gql_demo {

PathReference SearchedPath::pathReferenceValue() const {
  PathReference path;
  path.edgeIds.assign(edges.begin(), edges.end());
  path.nodeIds.assign(nodes.begin(), nodes.end());
  return path;
}

Graph::NodeId SearchedPath::currentNode() const {
  assert(!nodes.empty());
  return reverse_ ? nodes.front() : nodes.back();
}

Graph::EdgeId SearchedPath::currentEdge() const {
  assert(!edges.empty());
  return reverse_ ? edges.front() : edges.back();
}

bool SearchedPath::ReferenceCurrentNode() {
  if (matchedNodes) {
    auto [matchedIt, inserted] =
        matchedNodes->emplace(currentNode(), MatchedNode{});
    auto currentNodeIt = reverse_ ? nodes.begin() : std::prev(nodes.end());
    if (inserted) {
      matchedIt->second.count = 1;
      matchedIt->second.position = currentNodeIt;
    } else {
      if (matchedIt->second.position != currentNodeIt) {
        return false;
      }
      matchedIt->second.count++;
    }
  }
  return true;
}

void SearchedPath::UnreferenceCurrentNode() {
  if (matchedNodes) {
    auto nodeIt = matchedNodes->find(currentNode());
    assert(nodeIt != matchedNodes->end());
    assert((reverse_ ? nodes.begin() : std::prev(nodes.end())) ==
           nodeIt->second.position);
    if (--nodeIt->second.count == 0) {
      matchedNodes->erase(nodeIt);
    }
  }
}

void SearchedPath::PushNode(Graph::NodeId node) {
  assert(nodes.empty());
  assert(edges.empty());

  if (watchdog_) {
    watchdog_->Check();
  }

  nodes.push_back(node);
}

void SearchedPath::PopNode() {
  nodes.pop_back();
}

bool SearchedPath::PushEdgeNode(Graph::EdgeId edge, Graph::NodeId node) {
  if (watchdog_) {
    watchdog_->Check();
  }

  helpers::ScopeExit scopeExit;
  if (matchedEdges) {
    auto [matchedIt, inserted] = matchedEdges->insert(edge);
    if (!inserted) {
      return false;
    }
    scopeExit.Add([&, edge]() { matchedEdges->erase(matchedIt); });
  }
  if (boundEdges) {
    if (!boundEdges->insert(edge).second) {
      return false;
    }
  }
  scopeExit.Clear();

  if (reverse_) {
    nodes.push_front(node);
    edges.push_front(edge);
  } else {
    nodes.push_back(node);
    edges.push_back(edge);
  }
  return true;
}

void SearchedPath::PopEdgeNode() {
  if (matchedEdges) {
    matchedEdges->erase(currentEdge());
  }
  if (boundEdges) {
    boundEdges->erase(currentEdge());
  }

  if (reverse_) {
    nodes.pop_front();
    edges.pop_front();
  } else {
    nodes.pop_back();
    edges.pop_back();
  }
}

void SearchedPath::AssertEmpty() const {
  assert(nodes.empty());
  assert(edges.empty());
  assert(!matchedNodes);
  assert(!matchedEdges);
  assert(savedMatchedNodes.empty());
  assert(!reverse_);
}

SearchContext::SearchContext(
    const Graph& graph,
    const WorkingRecord& workingRecord,
    EvaluateSearchConditionFunc evaluateSearchCondition,
    size_t pathCount,
    bool isDifferentEdgesMode,
    TimeoutWatchdog* watchdog)
    : workingRecord(workingRecord),
      evaluateSearchCondition(evaluateSearchCondition),
      graph_(graph) {
  if (isDifferentEdgesMode) {
    boundEdges.emplace();
  }
  paths.reserve(pathCount);
  for (size_t i = 0; i < pathCount; i++) {
    paths.emplace_back(boundEdges);
    paths.back().SetTimeoutWatchdog(watchdog);
  }
}

PathReference SearchContext::pathReferenceValue(const std::string& var) const {
  if (auto it = pathVariableToIndex.find(var);
      it != pathVariableToIndex.end()) {
    return paths[it->second].pathReferenceValue();
  } else {
    assert(false);
    return {};
  }
}

void SearchContext::SetPathVariable(size_t pathIdx, const std::string& var) {
  pathVariableToIndex[var] = pathIdx;
}

Value SearchContext::GetElementBindingsValue(
    gql::GraphPatternVariableType varType,
    gql::VariableDegreeOfExposure degree,
    const ElementBindingsRange& bindingsRange) const {
  switch (varType) {
    case gql::GraphPatternVariableType::Node: {
      switch (degree) {
        case gql::VariableDegreeOfExposure::UnconditionalSingleton:
          assert(bindingsRange.second == 1);
          return {NodeReference{*bindingsRange.first}};
        case gql::VariableDegreeOfExposure::ConditionalSingleton:
          assert(bindingsRange.second <= 1);
          if (bindingsRange.second == 0) {
            return {Null{}};
          }
          return {NodeReference{*bindingsRange.first}};
        case gql::VariableDegreeOfExposure::EffectivelyBoundedGroup:
        case gql::VariableDegreeOfExposure::EffectivelyUnboundedGroup: {
          List nodeList;
          size_t i = 0;
          for (auto it = bindingsRange.first; i < bindingsRange.second;
               it++, i++) {
            nodeList.emplace_back()->v = NodeReference{*it};
          }
          return {std::move(nodeList)};
        }
        default:
          assert(false);
      }
      break;
    }
    case gql::GraphPatternVariableType::Edge: {
      switch (degree) {
        case gql::VariableDegreeOfExposure::UnconditionalSingleton:
          assert(bindingsRange.second == 1);
          return {EdgeReference{*bindingsRange.first}};
        case gql::VariableDegreeOfExposure::ConditionalSingleton:
          assert(bindingsRange.second <= 1);
          if (bindingsRange.second == 0) {
            return {Null{}};
          }
          return {EdgeReference{*bindingsRange.first}};
        case gql::VariableDegreeOfExposure::EffectivelyBoundedGroup:
        case gql::VariableDegreeOfExposure::EffectivelyUnboundedGroup: {
          List edgeList;
          size_t i = 0;
          for (auto it = bindingsRange.first; i < bindingsRange.second;
               it++, i++) {
            edgeList.emplace_back()->v = EdgeReference{*it};
          }
          return {std::move(edgeList)};
        }
        default:
          assert(false);
      }
      break;
    }
    default:
      break;
  }
  assert(false);
  return {};
}

bool SearchContext::VariableBindings::Bind(ElementBindingsRange bindings,
                                           bool isSingleton) {
  assert(isSingleton || !levels.top()->bindings);
  if (isSingleton) {
    if (levels.top()->bindings) {
      // Second appearance of unconditional singleton binding can't add any new
      // values.
      assert(bindings.second == 0);
      return false;
    }
    assert(bindings.second == 1);
  }
  levels.top()->bindings = bindings;
  return true;
}

std::list<Graph::EdgeOrNodeId>& SearchContext::boundElementVariable(
    const std::string& name) {
  auto& vars = boundElementVariables[name];
  if (!vars) {
    vars = std::make_unique<std::list<Graph::EdgeOrNodeId>>();
  }
  return *vars;
}

}  // namespace gql_demo