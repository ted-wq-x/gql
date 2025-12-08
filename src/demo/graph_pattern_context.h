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

#include <algorithm>
#include <stack>

#include "gql/ast/ast.h"
#include "gql/syntax_analyzer/aux_data.h"
#include "graph.h"

namespace gql_demo {

class SearchOp;
using SearchOpPtr = std::shared_ptr<SearchOp>;

// This class is used during plan preparation stage.
struct GraphPatternContext {
  GraphPatternContext();
  GraphPatternContext(const GraphPatternContext&) = default;
  ~GraphPatternContext();
  bool operator==(const GraphPatternContext&) const = default;
  GraphPatternContext& operator=(const GraphPatternContext&) = default;
  GraphPatternContext& operator=(GraphPatternContext&&) = default;

  // Variables that have unconditional singleton degree of reference in current
  // level of path pattern and are already bound.
  std::stack<std::unordered_set<std::string>> boundJoinableVariables;

  struct VariableBindings {
    struct Level {
      bool isBound = false;
    };
    using Reference = std::shared_ptr<const Level>;

    VariableBindings();
    ~VariableBindings();
    bool operator==(const VariableBindings&) const = default;
    void PushLevel();
    void PopLevel();
    void Bind(bool isSingleton);
    Reference reference() const;

   private:
    std::stack<std::shared_ptr<Level>> levels;
  };
  std::unordered_map<std::string, VariableBindings> variables;

  struct PendingCondition {
    SearchOpPtr op;
    std::vector<VariableBindings::Reference> referencedVars;

    bool isReadyToEvaluate() const;
    bool operator==(const PendingCondition&) const = default;
  };
  std::vector<PendingCondition> pendingConditions;
  size_t nextConditionIndex = 0;
};

struct PathPatternContext {
  PathPatternContext(GraphPatternContext& graph) : graph(graph) {}

  GraphPatternContext& graph;

  // Used to track nested path mode scopes.
  bool pathModeUniqueNodes = false;
  bool pathModeUniqueEdges = false;

  // When starting path processing from a known element in the middle of the
  // path pattern, it points to that element. It changes to nullptr when
  // processing moves past that element.
  const gql::ast::ElementPattern* startFromElement = nullptr;
  bool reverseDirection = false;
};

// Inline implementations

inline GraphPatternContext::GraphPatternContext() {
  boundJoinableVariables.emplace();
}

inline GraphPatternContext::~GraphPatternContext() {
  assert(boundJoinableVariables.size() == 1);
  assert(pendingConditions.empty());
}

inline GraphPatternContext::VariableBindings::VariableBindings() {
  PushLevel();
}

inline GraphPatternContext::VariableBindings::~VariableBindings() {
  assert(levels.size() == 1);
}

inline void GraphPatternContext::VariableBindings::PushLevel() {
  levels.push(std::make_shared<Level>());
}

inline void GraphPatternContext::VariableBindings::PopLevel() {
  levels.pop();
}

inline void GraphPatternContext::VariableBindings::Bind(bool isSingleton) {
  assert(isSingleton || !levels.top()->isBound);
  levels.top()->isBound = true;
}

inline GraphPatternContext::VariableBindings::Reference
GraphPatternContext::VariableBindings::reference() const {
  return levels.top();
}

inline bool GraphPatternContext::PendingCondition::isReadyToEvaluate() const {
  return std::all_of(referencedVars.begin(), referencedVars.end(),
                     [](const auto& vb) { return vb->isBound; });
}

}  // namespace gql_demo