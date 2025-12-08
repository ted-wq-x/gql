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

#include "graph_pattern_planner.h"
#include "operators.h"

namespace gql_demo {

class GraphPatternPlanner::VariableBindingsLevelScope {
 public:
  VariableBindingsLevelScope(
      GraphPatternContext&,
      const gql::syntax_analyzer::PathVariableReferenceScopeAuxData&,
      LinearOpSequence&);
  ~VariableBindingsLevelScope();

 private:
  GraphPatternContext& context_;
  const gql::syntax_analyzer::PathVariableReferenceScopeAuxData& auxData_;
  LinearOpSequence& opSequence_;
  SearchOpPtr exitOp_;
};

class GraphPatternPlanner::VariableBindingsScope {
 public:
  VariableBindingsScope(
      PathPatternContext&,
      const gql::syntax_analyzer::PathVariableReferenceScopeAuxData&,
      LinearOpSequence&);

  ~VariableBindingsScope();

 private:
  PathPatternContext& context_;
  const gql::syntax_analyzer::PathVariableReferenceScopeAuxData& auxData_;
  LinearOpSequence& opSequence_;
  const bool waitingForStartElement_;
  std::shared_ptr<VariableBindingsOp> enterOp_;
};

}  // namespace gql_demo