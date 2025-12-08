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

#include <memory>
#include <optional>

#include "gql/ast/detail/value_ptr.h"
#include "gql/ast/nodes/node.h"

namespace gql::ast {
struct BinarySetFunction;
struct FieldType;
struct GeneralSetFunction;
struct GQLProgram;
struct GraphPattern;
struct GraphPatternWhereClause;
struct ParenthesizedPathPatternWhereClause;
struct PathFactor;
struct PathPattern;
struct PathPatternExpression;
}  // namespace gql::ast

namespace gql::syntax_analyzer {

struct AggregateFunctionAuxData;
struct GraphPatternAuxData;
struct GraphPatternWhereClauseAuxData;
struct PathVariableReferenceScopeAuxData;
struct PathPatternAuxData;

#ifdef NDEBUG
using NodeBase = ast::Node;
#else
using RecordTypePtr = ast::value_ptr<std::vector<ast::FieldType> >;

struct NodeBase : ast::Node {
  // optional not set here means unknown or "diagnostics not available yet".
  std::optional<RecordTypePtr> debugOutgoingWorkingRecordType;
  // Can be statement declared type or type of outgoing working table.
  std::optional<RecordTypePtr> debugTableType;
};
#endif

#ifdef NDEBUG
using GQLProgramBase = ast::Node;
#else
using ExecutionOutcomeType = std::optional<RecordTypePtr>;

struct GQLProgramBase : ast::Node {
  ExecutionOutcomeType debugExecutionOutcomeType;
};
#endif

struct GraphPatternBase : NodeBase {
  std::shared_ptr<const GraphPatternAuxData> auxData;
};

struct PathPatternBase : NodeBase {
  std::shared_ptr<const PathPatternAuxData> auxData;
};

struct PathFactorBase : NodeBase {
  std::shared_ptr<const PathVariableReferenceScopeAuxData> auxData;
};

struct PathPatternExpressionBase : NodeBase {
  std::shared_ptr<const PathVariableReferenceScopeAuxData> auxData;
};

// Common base for classes holding expressions in graph pattern.
struct GraphPatternWhereClauseBase : NodeBase {
  std::shared_ptr<const GraphPatternWhereClauseAuxData> auxData;
};

struct AggregateFunctionBase : NodeBase {
  std::shared_ptr<const AggregateFunctionAuxData> auxData;
};

// |AstNodeBase| metafunction is used to select base type depending on node
// type.
template <typename T>
struct AstNodeBase {
  using type = NodeBase;
};

template <>
struct AstNodeBase<ast::GQLProgram> {
  using type = syntax_analyzer::GQLProgramBase;
};

template <>
struct AstNodeBase<ast::GraphPattern> {
  using type = syntax_analyzer::GraphPatternBase;
};

template <>
struct AstNodeBase<ast::PathPattern> {
  using type = syntax_analyzer::PathPatternBase;
};

template <>
struct AstNodeBase<ast::PathFactor> {
  using type = syntax_analyzer::PathFactorBase;
};

template <>
struct AstNodeBase<ast::PathPatternExpression> {
  using type = syntax_analyzer::PathPatternExpressionBase;
};

template <>
struct AstNodeBase<ast::GraphPatternWhereClause> {
  using type = syntax_analyzer::GraphPatternWhereClauseBase;
};

template <>
struct AstNodeBase<ast::ParenthesizedPathPatternWhereClause> {
  using type = syntax_analyzer::GraphPatternWhereClauseBase;
};

template <>
struct AstNodeBase<ast::GeneralSetFunction> {
  using type = syntax_analyzer::AggregateFunctionBase;
};

template <>
struct AstNodeBase<ast::BinarySetFunction> {
  using type = syntax_analyzer::AggregateFunctionBase;
};

}  // namespace gql::syntax_analyzer

namespace gql::ast {

template <typename T>
using NodeBase = typename syntax_analyzer::AstNodeBase<T>::type;

}  // namespace gql::ast
