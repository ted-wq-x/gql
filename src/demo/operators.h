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
#include <memory>
#include <optional>
#include <vector>

#include "gql/ast/ast.h"
#include "gql/syntax_analyzer/aux_data.h"
#include "graph.h"
#include "search_context.h"

namespace gql_demo {

class SearchOp;
using SearchOpPtr = std::shared_ptr<SearchOp>;

class SearchOp {
 public:
  void SetNextOp(SearchOpPtr);

  // Execute() must leave context in the same state as it was when called.
  // In some cases SearchOp may change it's own state (e.g. QuantifiedOp).
  // Execute() must be reentrant - Execute() of same SearchOp instance may be
  // called down the call stack (e.g. inside repeating pattern).
  virtual void Execute(SearchContext&) = 0;

 protected:
  SearchOp() = default;
  SearchOp(const SearchOp&) = delete;
  virtual ~SearchOp() {}

  SearchOpPtr nextOp;
};

class ScanAllNodesOp : public SearchOp {
 public:
  void Execute(SearchContext& context) override;
};

// Same QuantifiedOp appears multiple times in the execution stack
class QuantifiedOp : public SearchOp {
 public:
  SearchOpPtr loopOp;

  QuantifiedOp(size_t lowerCount, std::optional<size_t> upperCount)
      : lowerCount(lowerCount), upperCount(upperCount) {}

  void Execute(SearchContext& context) override;

 private:
  const size_t lowerCount = 0;
  const std::optional<size_t> upperCount;
  size_t counter = 0;
};

class ForkOp : public SearchOp {
 public:
  std::vector<SearchOpPtr> nextOps;

  void Execute(SearchContext& context) override {
    for (auto& op : nextOps) {
      op->Execute(context);
    }
  }
};

class NoOp : public SearchOp {
 public:
  void Execute(SearchContext& context) override { nextOp->Execute(context); }
};

class GoToOp : public SearchOp {
 public:
  explicit GoToOp(SearchOpPtr op) : targetOp(std::move(op)) {}

  void Execute(SearchContext& context) override {
    if (auto op = targetOp.lock()) {
      op->Execute(context);
    }
  }

 private:
  std::weak_ptr<SearchOp> targetOp;
};

class ExpandEdgeOp : public SearchOp {
 public:
  explicit ExpandEdgeOp(gql::ast::EdgeDirectionPattern direction);

  void JoinEdge(const std::string& var);
  void SetLabelExpression(const gql::ast::LabelExpression& expr);

  void Execute(SearchContext& context) override;

 private:
  const gql::ast::EdgeDirectionPattern direction;
  std::optional<gql::ast::LabelExpression> labelExpression;
  std::string joinVar;

  bool CheckEdge(SearchContext& context,
                 const Graph::Edge&,
                 Graph::NodeId currentNodeId) const;
  bool CheckDirection(std::optional<bool> rightDirection) const;
};

class JoinNodeOp : public SearchOp {
 public:
  JoinNodeOp(const std::string& var) : var(var) {}

  void Execute(SearchContext& context) override;

 private:
  const std::string var;
};

// See comments for SearchedPath::ReferenceCurrentNode() for details.
class ReferenceCurrentNodeOp : public SearchOp {
 public:
  void Execute(SearchContext& context) override;
};

class BindElementOp : public SearchOp {
 public:
  BindElementOp(const std::string& var, bool isNode);

  void Execute(SearchContext& context) override;

 private:
  const std::string var;
  const bool isNode;
};

class VariableBindingsLevelOp : public SearchOp {
 public:
  std::shared_ptr<SearchOp> CreateExitOp();

  void Execute(SearchContext& context) override;

  std::vector<std::string> variables;

 private:
  bool startOp = true;
};

// Saves new bindings of specified variables that are added between the op and
// exitOp counterpart to the current level of variable reference.
class VariableBindingsOp : public SearchOp {
 public:
  std::shared_ptr<VariableBindingsOp> CreateExitOp();

  void Execute(SearchContext& context) override;

  std::vector<std::pair<std::string, bool>>
      variables;  // second - is unconditional singleton

 private:
  bool startOp = true;
  struct SharedState {
    std::vector<size_t> bindingsStartSize;
  };
  std::shared_ptr<SharedState> sharedState;
};

class AddConditionClosureOp : public SearchOp {
 public:
  explicit AddConditionClosureOp(size_t closureIdx);

  void Execute(SearchContext& context) override;

  std::vector<std::string> variables;

 private:
  const size_t closureIdx;
};

class CheckConditionOp : public SearchOp {
 public:
  CheckConditionOp(const gql::ast::ValueExpression&,
                   const gql::syntax_analyzer::GraphPatternWhereClauseAuxData&
                       conditionAuxData,
                   size_t closureIdx);

  void Execute(SearchContext& context) override;

 private:
  const gql::ast::ValueExpression condition;
  const gql::syntax_analyzer::GraphPatternWhereClauseAuxData conditionAuxData;
  const size_t closureIdx;
};

// Also sets path direction to forward.
class SetCurrentPathOp : public SearchOp {
 public:
  explicit SetCurrentPathOp(size_t pathIndex) : pathIndex(pathIndex) {}

  void Execute(SearchContext& context) override;

 private:
  const size_t pathIndex;
};

class SetCurrentPathReverseDirectionOp : public SearchOp {
 public:
  explicit SetCurrentPathReverseDirectionOp(bool reverse = true)
      : reverse(reverse) {}

  void Execute(SearchContext& context) override;

 private:
  const bool reverse;
};

class SaveMatchOp : public SearchOp {
 public:
  explicit SaveMatchOp(std::function<void(const SearchContext&)> callback)
      : callback(std::move(callback)) {}

  void Execute(SearchContext& context) override {
    assert(!nextOp);
    callback(context);
  }

 private:
  std::function<void(const SearchContext&)> callback;
};

class CheckNodeLabelsOp : public SearchOp {
 public:
  explicit CheckNodeLabelsOp(const gql::ast::LabelExpression& labelExpression)
      : labelExpression(labelExpression) {}

  void Execute(SearchContext& context) override;

 private:
  const gql::ast::LabelExpression labelExpression;
};

class PathModeOpState {
 public:
  void SetSaveStateOnExit() { saveStateOnExit = true; }
  void SetRestoreStateOnEnter() { restoreStateOnEnter = true; }

 protected:
  bool saveStateOnExit = false;
  bool restoreStateOnEnter = false;
};

// Starts and ends TRAIL path mode
class PathModeUniqueEdgesOp : public SearchOp, public PathModeOpState {
 public:
  explicit PathModeUniqueEdgesOp(bool startOp) : startOp(startOp) {}

  void Execute(SearchContext& context) override;

 private:
  const bool startOp;
};

// Starts and ends ACYCLIC path mode
class PathModeUniqueNodesOp : public SearchOp, public PathModeOpState {
 public:
  explicit PathModeUniqueNodesOp(bool startOp) : startOp(startOp) {}

  void Execute(SearchContext& context) override;

 private:
  const bool startOp;
};

}  // namespace gql_demo