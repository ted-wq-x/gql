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

#include <stack>

#include "binding_table.h"
#include "gql/ast/ast.h"
#include "graph.h"

namespace gql_demo {

class TimeoutWatchdog;

class SearchOp;
using SearchOpPtr = std::shared_ptr<SearchOp>;

using OptBindingTable = std::optional<BindingTable>;

class Demo {
 public:
  explicit Demo(Graph& graph) : graph(graph) {}

  void SetTimeoutWatchdog(TimeoutWatchdog& watchdog) { watchdog_ = &watchdog; }
  OptBindingTable Process(const gql::ast::GQLProgram&);

  class ExecutionContext {
   public:
    ExecutionContext MakeCopy() const { return *this; }
    ExecutionContext MakeChild() const;
    ExecutionContext MakeAmended(size_t index) const;

    WorkingRecord workingRecord;
    BindingTable workingTable = BindingTable::Unit();

   private:
    ExecutionContext& operator=(const ExecutionContext&) = default;
  };

 private:
  class ContextStateChecker;

  OptBindingTable Process(const gql::ast::ProcedureBody&, ExecutionContext&);
  OptBindingTable Process(const gql::ast::Statement&, ExecutionContext&);
  void Process(const gql::ast::YieldClause&, ExecutionContext&);
  void Process(const gql::ast::CallProcedureStatement&, ExecutionContext&);
  OptBindingTable Process(const gql::ast::LinearDataModifyingStatementBody&,
                          ExecutionContext&);
  void Process(const gql::ast::PrimitiveDataModifyingStatement&,
               ExecutionContext&);
  void Process(const gql::ast::InsertStatement&, ExecutionContext&);
  void Process(const gql::ast::DeleteStatement&, ExecutionContext&);
  BindingTable Process(const gql::ast::CompositeQueryStatement&,
                       ExecutionContext&);
  BindingTable Process(const gql::ast::CompositeQueryPrimary&,
                       ExecutionContext&);
  void Process(const gql::ast::SimpleQueryStatement&, ExecutionContext&);
  void Process(const gql::ast::SimpleMatchStatement&, ExecutionContext&);

  BindingTable Process(const gql::ast::PrimitiveResultStatement&,
                       ExecutionContext&);
  void Process(const gql::ast::FilterStatement&, ExecutionContext&);
  void Process(const gql::ast::ForStatement&, ExecutionContext&);
  BindingTable Process(const gql::ast::ResultStatement&, ExecutionContext&);
  Value Evaluate(const gql::ast::ValueExpression&, const ExecutionContext&);
  Value Evaluate(const gql::ast::BindingVariableBase&, const ExecutionContext&);
  Value Evaluate(const gql::ast::UnsignedLiteral&, const ExecutionContext&);
  Value Evaluate(const gql::ast::ValueExpression::BooleanTest&,
                 const ExecutionContext&);
  Value Evaluate(const gql::ast::ValueExpression::Unary&,
                 const ExecutionContext&);
  Value Evaluate(const gql::ast::ValueExpression::Binary&,
                 const ExecutionContext&);
  Value Evaluate(const gql::ast::ValueExpression::Comparison&,
                 const ExecutionContext&);
  Value Evaluate(const gql::ast::PropertyReference&, const ExecutionContext&);
  Value Evaluate(const gql::ast::PropertyExistsPredicate&,
                 const ExecutionContext&);
  Value Evaluate(const gql::ast::DirectedPredicate&, const ExecutionContext&);
  Value Evaluate(const gql::ast::LabeledPredicate&, const ExecutionContext&);
  Value Evaluate(const gql::ast::SourceDestinationPredicate&,
                 const ExecutionContext&);
  Value Evaluate(const gql::ast::AllElementsPredicate&,
                 const ExecutionContext&);
  Value Evaluate(const gql::ast::ElementIdFunction&, const ExecutionContext&);
  Value Evaluate(const gql::ast::AggregateFunction&, const ExecutionContext&);
  Value Evaluate(const gql::ast::LetValueExpression&, const ExecutionContext&);
  Value EvaluateValueQueryExpression(const gql::ast::ProcedureBody&,
                                     const ExecutionContext&);
  Value Evaluate(const gql::ast::BindingTableExpression&,
                 const ExecutionContext&);
  Value Evaluate(const gql::ast::CaseExpression& expr,
                 const ExecutionContext& context);

  Graph::Element* FindElementByVariable(
      const gql::ast::BindingVariableReference&,
      const ExecutionContext&);

  template <typename NodeType>
  auto ProcessAndCheckType(const NodeType&, ExecutionContext&);

  Graph& graph;
  TimeoutWatchdog* watchdog_ = nullptr;
};

// In Release mode ContextStateChecker does nothing. In Debug mode it checks
// that resulting working table and working record types match expected.
class Demo::ContextStateChecker {
 public:
#ifdef NDEBUG
  ContextStateChecker(const gql::syntax_analyzer::NodeBase&,
                      const ExecutionContext&) {}
#else
  ContextStateChecker(const gql::syntax_analyzer::NodeBase&,
                      const ExecutionContext&);
  ~ContextStateChecker();

  static void CheckTableType(const BindingTable&,
                             const gql::ast::FieldTypeList&);
  static void CheckRecordType(const WorkingRecord&,
                              const gql::ast::FieldTypeList&);

 private:
  const gql::syntax_analyzer::NodeBase& node_;
  const ExecutionContext& context_;
#endif
};

inline Demo::ExecutionContext Demo::ExecutionContext::MakeChild() const {
  ExecutionContext childContext;
  childContext.workingRecord.parentRecord = &workingRecord;
  return childContext;
}

inline Demo::ExecutionContext Demo::ExecutionContext::MakeAmended(
    size_t index) const {
  ExecutionContext childContext;
  childContext.workingRecord.parentRecord = &workingRecord;
  childContext.workingRecord.table = &workingTable;
  childContext.workingRecord.index = index;
  return childContext;
}

// In Release mode ProcessAndCheckType just calls Process. In Debug mode it also
// checks that resulting working table and working record types match expected.
template <typename NodeType>
auto Demo::ProcessAndCheckType(const NodeType& node,
                               ExecutionContext& context) {
  auto resultType = Process(node, context);
#ifndef NDEBUG
  if (node.debugOutgoingWorkingRecordType) {
    ContextStateChecker::CheckRecordType(context.workingRecord,
                                         **node.debugOutgoingWorkingRecordType);
  }
  if (node.debugTableType) {
    if constexpr (std::is_same_v<decltype(resultType), OptBindingTable>) {
      assert(resultType.has_value());
      ContextStateChecker::CheckTableType(*resultType, **node.debugTableType);
    } else {
      ContextStateChecker::CheckTableType(resultType, **node.debugTableType);
    }
  }
#endif
  return resultType;
}

}  // namespace gql_demo