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

#include <unordered_set>

#include "gql/ast/ast.h"

namespace gql_demo {

class LinearOpSequence;
struct GraphPatternContext;
struct PathPatternContext;

class GraphPatternPlanner {
 public:
  static void CreateExecutionPlan(
      const gql::ast::SimpleMatchStatement&,
      LinearOpSequence&,
      const std::unordered_set<std::string>& overlapFields);

 private:
  static void ProcessPathJoinedOnVar(const gql::ast::PathPattern&,
                                     const std::string& var,
                                     GraphPatternContext&,
                                     LinearOpSequence&);
  static void Process(const gql::ast::PathPatternExpression&,
                      PathPatternContext&,
                      LinearOpSequence&);
  static void AddCheckCondition(
      GraphPatternContext&,
      LinearOpSequence&,
      const gql::ast::SearchCondition&,
      const gql::syntax_analyzer::GraphPatternWhereClauseAuxData&);
  static void MaybeAppendCheckCondition(GraphPatternContext&,
                                        LinearOpSequence&);

  class VariableBindingsScope;
  class VariableBindingsLevelScope;
};

}  // namespace gql_demo