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

#include "syntax_analyzer.h"

#include <unordered_set>

#include "common/formatted_error.h"
#include "gql/ast/algorithm.h"
#include "match_utils.h"
#include "type_helpers.h"

namespace gql {

void SyntaxAnalyzer::Process(ast::SimpleMatchStatement& statement,
                             ExecutionContext& context) {
  ContextStateSaver contextStateSaver(statement, context);

  auto bindingTable = ProcessGraphPatternBindingTable(statement, context);
  // 14.4 Syntax Rule 5
  for (auto& f : bindingTable) {
    if (auto* field = HasField(context.workingRecord, f.name.name)) {
      *field = f;
    } else if (auto* tableField = HasField(context.workingTable, f.name.name)) {
      *tableField = f;
    } else {
      context.workingTable.push_back(f);
    }
  }
}

ast::CallProcedureStatement SyntaxAnalyzer::Rewrite(
    const ast::OptionalMatchStatement& statement,
    const ExecutionContext& context) const {
  ast::CallProcedureStatement callProc;
  callProc.isOptional = true;
  auto& proc = callProc.call.emplace<ast::InlineProcedureCall>();
  auto& varScope = proc.vars.emplace();
  for (auto& f : context.workingTable) {
    varScope.emplace_back().name = f.name.name;
  }
  for (auto& f : context.workingRecord) {
    varScope.emplace_back().name = f.name.name;
  }
  auto& linearQuery = proc.spec->statements.firstStatement->option
                          .emplace<ast::CompositeQueryStatement>()
                          .queries.emplace_back()
                          .emplace<ast::LinearQueryStatementOption>();
  auto& queries =
      linearQuery.statements.emplace<ast::SimpleLinearQueryStatement>();

  for (auto& stmt : statement.statements->statements) {
    queries.emplace_back(stmt);
  }

  ast::ReturnItemList& returnItems =
      linearQuery.result.option.emplace<ast::ResultStatement>().items.emplace();

  std::unordered_set<std::string> cols;
  detail::CollectMatchOutputColumns(statement, cols);
  for (auto& col : cols) {
    if (HasField(context.workingRecord, col) ||
        HasField(context.workingTable, col)) {
      continue;
    }
    returnItems.emplace_back()
        .aggregate.option.emplace<ast::BindingVariableReference>()
        .name = col;
  }
  return callProc;
}

}  // namespace gql