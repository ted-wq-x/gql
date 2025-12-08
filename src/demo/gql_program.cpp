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

#include "demo.h"

#include <cassert>
#include "gql/ast/algorithm.h"

namespace gql_demo {

OptBindingTable Demo::Process(const gql::ast::GQLProgram& program) {
  OptBindingTable outcome;
  ExecutionContext context;
  gql::ast::variant_switch(
      program.programActivity,
      [&](const gql::ast::SessionActivity& value) {
        for (auto& setCmd : value.setCommands) {
          gql::ast::variant_switch(
              setCmd,
              [&](const gql::ast::SessionSetSchemaClause&) {
                // TODO: Implement
              },
              [&](const gql::ast::SessionSetGraphClause&) {
                // Feature disabled
                assert(false);
              },
              [&](const gql::ast::SessionSetTimeZoneClause&) {
                // Feature disabled
                assert(false);
              },
              [&](const gql::ast::SessionSetParameterClause&) {
                // Feature disabled
                assert(false);
              });
        }
      },
      [&](const gql::ast::TransactionActivity& transaction) {
        if (transaction.procedure) {
          outcome = ProcessAndCheckType(*transaction.procedure, context);
        }
      });
  if (program.sessionCloseCommand) {
    outcome.reset();
  }
#ifndef NDEBUG
  assert(program.debugExecutionOutcomeType.has_value() == outcome.has_value());
  if (outcome.has_value()) {
    ContextStateChecker::CheckTableType(*outcome,
                                        **program.debugExecutionOutcomeType);
  }
#endif
  return outcome;
}

OptBindingTable Demo::Process(const gql::ast::ProcedureBody& procDef,
                              ExecutionContext& context) {
  BindingTable localVars;
  // childContext includes variables from variable definition block.
  auto childContext = context.MakeChild();
  childContext.workingRecord.table = &localVars;
  childContext.workingRecord.index = 0;
  auto inserter = localVars.CreateColumnInserter();
  for (auto& varDef : procDef.vars) {
    gql::ast::variant_switch(
        varDef,
        [&](const gql::ast::GraphVariableDefinition&) {
          // Feature disabled
          assert(false);
        },
        [&](const gql::ast::BindingTableVariableDefinition& expr) {
          inserter.Set(expr.var.name,
                       Evaluate(expr.initializer.initializer, childContext));
          // TODO: Check type
        },
        [&](const gql::ast::ValueVariableDefinition& expr) {
          inserter.Set(expr.var.name,
                       Evaluate(*expr.initializer.initializer, childContext));
        });
  }
  inserter.NextRow();
  OptBindingTable outcome =
      ProcessAndCheckType(*procDef.statements.firstStatement, childContext);
  for (auto& stmt : procDef.statements.nextStatements) {
    if (outcome) {
      context.workingTable = std::move(*outcome);
    } else {
      context.workingTable = BindingTable::Unit();
    }
    if (!stmt.yield.empty()) {
      Process(stmt.yield, childContext);
    }

    outcome = ProcessAndCheckType(*stmt.statement, childContext);
  }
  return outcome;
}

OptBindingTable Demo::Process(const gql::ast::Statement& statement,
                              ExecutionContext& context) {
  return gql::ast::variant_switch(
      statement.option,
      [&](const gql::ast::LinearCatalogModifyingStatement&) {
        // Feature disabled
        assert(false);
        return OptBindingTable{};
      },
      [&](const gql::ast::LinearDataModifyingStatement& dataModifyingStmt) {
        ContextStateChecker contextStateChecker(dataModifyingStmt, context);

        return gql::ast::variant_switch(
            dataModifyingStmt.option,
            [&](const gql::ast::LinearDataModifyingStatementBody& statement) {
              return ProcessAndCheckType(statement, context);
            },
            [&](const gql::ast::ProcedureBody& statement) {
              ProcessAndCheckType(statement, context);
              return OptBindingTable{};
            });
      },
      [&](const gql::ast::CompositeQueryStatement& statement)
          -> OptBindingTable {
        return ProcessAndCheckType(statement, context);
      });
}

void Demo::Process(const gql::ast::YieldClause& yield,
                   ExecutionContext& context) {
  std::vector<std::string> yieldNames;
  for (auto& item : yield) {
    yieldNames.push_back(item.name.name);
  }
  context.workingTable =
      std::move(context.workingTable).Yield(yieldNames, context.workingRecord);
}

void Demo::Process(const gql::ast::CallProcedureStatement& statement,
                   ExecutionContext& context) {
  ContextStateChecker contextStateChecker(statement, context);

  gql::ast::variant_switch(
      statement.call,
      [&](const gql::ast::NamedProcedureCall&) {
        // Feature disabled
        assert(false);
      },
      [&](const gql::ast::InlineProcedureCall& inlineCall) {
        std::unordered_set<std::string> varScope;
        if (inlineCall.vars) {
          for (auto& v : *inlineCall.vars) {
            varScope.insert(v.name);
          }
        }

        BindingTable newTable;
        for (size_t i = 0; i < context.workingTable.size(); i++) {
          auto childContext = context.MakeAmended(i);
          if (inlineCall.vars) {
            childContext.workingRecord.SetFilter(varScope);
          }

          if (auto result =
                  ProcessAndCheckType(*inlineCall.spec, childContext)) {
            newTable.AppendProduct(context.workingTable, i, *result,
                                   statement.isOptional);
          }
        }
        context.workingTable = std::move(newTable);
      });
}

}  // namespace gql_demo
