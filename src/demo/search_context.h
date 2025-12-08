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

#include <functional>
#include <list>
#include <optional>
#include <stack>
#include <unordered_set>
#include <vector>


#include "gql/syntax_analyzer/defs.h"
#include "graph.h"
#include "value.h"

namespace gql::ast {
struct ValueExpression;
}

namespace gql_demo {

class WorkingRecord;
class TimeoutWatchdog;

class SearchedPath {
 public:
  SearchedPath(std::optional<std::unordered_set<Graph::EdgeId>>& boundEdges)
      : boundEdges(boundEdges) {}
  SearchedPath(SearchedPath&&) = default;
  SearchedPath(const SearchedPath&) = delete;
  void operator=(const SearchedPath&) = delete;

  void SetTimeoutWatchdog(TimeoutWatchdog* watchdog) { watchdog_ = watchdog; }

  PathReference pathReferenceValue() const;

  bool empty() const { return nodes.empty(); }
  bool reverseDirection() const { return reverse_; }
  Graph::NodeId currentNode() const;
  Graph::EdgeId currentEdge() const;

  void SetReverseDirection(bool reverse) { reverse_ = reverse; }
  void PushNode(Graph::NodeId);
  void PopNode();
  bool PushEdgeNode(Graph::EdgeId, Graph::NodeId);
  void PopEdgeNode();

  // Same node may be referenced multiple times in different restrictive
  // contexts. That's why it's not enough to just check on the node creation.
  // E.g. "(ACYCLIC ...-())()(ACYCLIC ()-...)" - three node patterns shown are
  // bind to the same node but in different restrictive contexts.
  // Another case is when we create edge we create node on the end of the edge
  // but this node may belong to a different restrictive context.
  // E.g. "...-(ACYCLIC ()...)" - shown node is created as part of edge but it
  // belongs to ACYCLIC context.
  bool ReferenceCurrentNode();
  void UnreferenceCurrentNode();

  void AssertEmpty() const;

 private:
  friend class PathModeUniqueNodesOp;
  friend class PathModeUniqueEdgesOp;

  // For DIFFERENT EDGES match mode
  std::optional<std::unordered_set<Graph::EdgeId>>& boundEdges;

  // Direction in which elements are currently added to the path
  bool reverse_ = false;

  // Path itself
  std::list<Graph::NodeId> nodes;
  std::list<Graph::EdgeId> edges;

  struct MatchedNode {
    int count = 0;
    std::list<Graph::NodeId>::const_iterator position;
  };
  // For ACYCLIC path mode
  using MatchedNodes = std::unordered_map<Graph::NodeId, MatchedNode>;
  std::optional<MatchedNodes> matchedNodes;
  MatchedNodes savedMatchedNodes;
  // For TRAIL path mode
  using MatchedEdges = std::unordered_set<Graph::EdgeId>;
  std::optional<MatchedEdges> matchedEdges;
  MatchedEdges savedMatchedEdges;

  TimeoutWatchdog* watchdog_ = nullptr;
};

class SearchContext {
 public:
  using EvaluateSearchConditionFunc =
      std::function<bool(const gql::ast::ValueExpression&,
                         const WorkingRecord&)>;

  SearchContext(const Graph& graph,
                const WorkingRecord&,
                EvaluateSearchConditionFunc,
                size_t pathCount,
                bool isDifferentEdgesMode,
                TimeoutWatchdog* watchdog);

  SearchedPath& currentPath() { return paths[currentPathIndex]; }
  const SearchedPath& path(size_t idx) const { return paths[idx]; }
  const Graph& graph() const { return graph_; }
  PathReference pathReferenceValue(const std::string&) const;
  void SetPathVariable(size_t pathIdx, const std::string&);
  std::list<Graph::EdgeOrNodeId>& boundElementVariable(const std::string& name);

  // <search condition>s don't need fields from the working record
  // that overlap with graph pattern variables. But it's checked by Syntax
  // Analyzer and doesn't need to be enforced here.
  const WorkingRecord& workingRecord;

  const EvaluateSearchConditionFunc evaluateSearchCondition;

  // Contains start iterator and size of the range.
  using ElementBindingsRange =
      std::pair<std::list<Graph::EdgeOrNodeId>::const_iterator, size_t>;

  Value GetElementBindingsValue(gql::GraphPatternVariableType,
                                gql::VariableDegreeOfExposure,
                                const ElementBindingsRange&) const;

  // Element variable bindings at the current level of path pattern matching.
  struct VariableBindings {
    struct Level {
      std::optional<ElementBindingsRange> bindings;
    };
    using Reference = std::shared_ptr<const Level>;

    VariableBindings() { PushLevel(); }
    ~VariableBindings() { assert(levels.size() == 1); }
    void PushLevel() { levels.push(std::make_shared<Level>()); }
    void PopLevel() { levels.pop(); }
    bool Bind(ElementBindingsRange bindings, bool isSingleton);
    void ResetBindings() { levels.top()->bindings.reset(); }
    Reference reference() const { return levels.top(); }

   private:
    std::stack<std::shared_ptr<Level>> levels;
  };
  std::unordered_map<std::string, VariableBindings> currentBindings;

  struct SearchConditionClosure {
    std::unordered_map<std::string, VariableBindings::Reference>
        variableBindings;
  };
  std::vector<std::vector<SearchConditionClosure>> searchConditionClosures;

 private:
  const Graph& graph_;

  size_t currentPathIndex = 0;
  std::vector<SearchedPath> paths;
  std::unordered_map<std::string, size_t> pathVariableToIndex;

  // All bound element variables from the graph pattern.
  std::unordered_map<std::string,
                     std::unique_ptr<std::list<Graph::EdgeOrNodeId>>>
      boundElementVariables;

  // For DIFFERENT EDGES match mode
  std::optional<std::unordered_set<Graph::EdgeId>> boundEdges;

  friend class SetCurrentPathOp;
};

}  // namespace gql_demo