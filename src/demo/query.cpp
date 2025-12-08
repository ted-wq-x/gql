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

#include "error.h"
#include "gql/ast/algorithm.h"

namespace gql_demo {

BindingTable Demo::Process(const gql::ast::CompositeQueryStatement& stmt,
                           ExecutionContext& context) {
  std::optional<BindingTable> lastResult;
  for (auto& query : stmt.queries) {
    auto childContext = context.MakeCopy();
    auto newTable = Process(query, childContext);
    if (!lastResult) {
      lastResult = newTable;
    } else {
      if (auto* op = std::get_if<gql::ast::SetOperator>(&stmt.conjunction)) {
        lastResult = TableConjunction(*lastResult, *op, newTable);
      } else {
        // OtherwiseConjunctionValue
        if (lastResult->size() == 0) {
          lastResult = newTable;
        }
      }
    }
  }
  context.workingTable = *lastResult;
  return *lastResult;
}

BindingTable Demo::Process(const gql::ast::CompositeQueryPrimary& query,
                           ExecutionContext& context) {
  BindingTable result;
  gql::ast::variant_switch(
      query,
      [&](const gql::ast::LinearQueryStatementOption& statement) {
        gql::ast::variant_switch(
            statement.statements,
            [&](const gql::ast::SimpleLinearQueryStatement& statement) {
              for (auto& simpleStmt : statement) {
                Process(simpleStmt, context);
              }
            },
            [&](const std::vector<gql::ast::FocusedLinearQueryStatementPart>&) {
              // Feature disabled
              assert(false);
            });
        result = ProcessAndCheckType(statement.result, context);
      },
      [&](const gql::ast::NestedQuerySpecification& statement) {
        assert(!statement.useGraph);

        ProcessAndCheckType(statement.procedure, context);
      },
      [&](const gql::ast::FocusedPrimitiveResultStatement&) { assert(false); },
      [&](const gql::ast::SelectStatement&) {
        // Shall be rewritten.
        assert(false);
      });
  return result;
}

void Demo::Process(const gql::ast::SimpleQueryStatement& simpleStmt,
                   ExecutionContext& context) {
  gql::ast::variant_switch(
      simpleStmt,
      [&](const gql::ast::MatchStatement& statement) {
        gql::ast::variant_switch(
            statement,
            [&](const gql::ast::SimpleMatchStatement& statement) {
              Process(statement, context);
            },
            [&](const gql::ast::OptionalMatchStatement&) { assert(false); });
      },
      [&](const gql::ast::LetStatement&) {
        // Shall be rewritten.
        assert(false);
      },
      [&](const gql::ast::ForStatement& statement) {
        Process(statement, context);
      },
      [&](const gql::ast::FilterStatement& statement) {
        Process(statement, context);
      },
      [&](const gql::ast::OrderByAndPageStatement& statement) {
        Process(statement, context);
      },
      [&](const gql::ast::CallProcedureStatement& statement) {
        Process(statement, context);
      });
}

BindingTable Demo::Process(const gql::ast::PrimitiveResultStatement& result,
                           ExecutionContext& context) {
  return gql::ast::variant_switch(
      result.option,
      [&](const gql::ast::FinishValue&) { return BindingTable{}; },
      [&](const gql::ast::ResultStatement& statement) {
        // TODO: Handle statement.orderByAndPage->orderBy

        BindingTable resultTable = ProcessAndCheckType(statement, context);

        if (statement.quantifier == gql::ast::SetQuantifier::DISTINCT) {
          resultTable.KeepDistinctRows();
        }

        if (statement.orderByAndPage) {
          size_t offset = 0;
          size_t limit = context.workingTable.size();
          if (statement.orderByAndPage->offset) {
            offset = std::get<gql::ast::UnsignedInteger>(
                *statement.orderByAndPage->offset);
          }
          if (statement.orderByAndPage->limit) {
            limit = std::get<gql::ast::UnsignedInteger>(
                *statement.orderByAndPage->limit);
          }
          resultTable.Slice(offset, limit);
        }

        return resultTable;
      });
}

void Demo::Process(const gql::ast::FilterStatement& stmt,
                   ExecutionContext& context) {
  ContextStateChecker contextStateChecker(stmt, context);

  for (size_t i = 0; i < context.workingTable.size();) {
    auto childContext = context.MakeAmended(i);
    auto value = Evaluate(*stmt.condition, childContext);
    if (auto* boolPtr = std::get_if<bool>(&value.v)) {
      if (*boolPtr) {
        i++;
        continue;
      }
    }
    context.workingTable.RemoveRow(i);
  }
}

BindingTable Demo::Process(const gql::ast::ResultStatement& statement,
                           ExecutionContext& context) {
  BindingTable newTable;
  for (auto& returnItem : *statement.items) {
    newTable.AddColumn(returnItem.alias->name, SetPreferredColumnSequence);
  }

  if (statement.groupBy.empty()) {
    // No GROUP BY clause
    auto inserter = newTable.CreateInserter();
    inserter.Reserve(context.workingTable.size());
    for (size_t rowIdx = 0; rowIdx < context.workingTable.size(); ++rowIdx) {
      auto childContext = context.MakeAmended(rowIdx);
      for (auto& returnItem : *statement.items) {
        inserter.Set(returnItem.alias->name,
                     Evaluate(returnItem.aggregate, childContext));
      }
      inserter.NextRow();
    }
    return newTable;
  }

  // GROUP BY clause is present

  // Items from return item list referenced in GROUP BY clause.
  std::unordered_map<std::string, const gql::ast::ValueExpression*>
      groupingReturnItems;
  // All columns in GROUP BY clause.
  std::unordered_set<std::string> groupCols;
  for (auto groupCol : statement.groupBy) {
    groupCols.insert(groupCol.name);
    auto returnItemIt =
        std::find_if(statement.items->begin(), statement.items->end(),
                     [&groupCol](const auto& returnItem) {
                       return returnItem.alias->name == groupCol.name;
                     });
    if (returnItemIt != statement.items->end()) {
      groupingReturnItems[groupCol.name] = &returnItemIt->aggregate;
    }
  }

  using GroupKey = BindingTable;
  std::vector<std::pair<GroupKey, BindingTable>> groups;
  for (size_t rowIdx = 0; rowIdx < context.workingTable.size(); ++rowIdx) {
    auto childContext = context.MakeAmended(rowIdx);
    GroupKey groupKey;
    auto inserter = groupKey.CreateColumnInserter();
    for (auto groupCol : statement.groupBy) {
      if (auto retIt = groupingReturnItems.find(groupCol.name);
          retIt != groupingReturnItems.end()) {
        inserter.Set(groupCol.name, Evaluate(*retIt->second, childContext));
      } else {
        inserter.Set(groupCol.name, Evaluate(groupCol, childContext));
      }
    }
    inserter.NextRow();

    BindingTable* groupTable = nullptr;
    for (auto& [key, grp] : groups) {
      if (key == groupKey) {
        groupTable = &grp;
        break;
      }
    }
    if (!groupTable) {
      groups.emplace_back(groupKey, BindingTable{});
      groupTable = &groups.back().second;
    }

    auto groupTableInserter = groupTable->CreateColumnInserter();
    for (auto& [colName, colData] : context.workingTable.columns()) {
      if (!groupCols.count(colName)) {
        groupTableInserter.Set(colName, colData[rowIdx]);
      }
    }
    // There can be no non-grouping columns that's why it's important to
    // call NextRow() to create a (empty) row in the table.
    groupTableInserter.NextRow();
  }

  auto inserter = newTable.CreateInserter();
  inserter.Reserve(groups.size());
  for (auto& [groupKey, groupTable] : groups) {
    auto aggregatingContext = context.MakeChild();
    aggregatingContext.workingTable = std::move(groupTable);
    aggregatingContext.workingRecord.table = &groupKey;
    aggregatingContext.workingRecord.index = 0;
    for (auto& returnItem : *statement.items) {
      Value value;
      if (auto colIt = groupKey.columns().find(returnItem.alias->name);
          colIt != groupKey.columns().end()) {
        value = colIt->second.front();
      } else {
        value = Evaluate(returnItem.aggregate, aggregatingContext);
      }
      inserter.Set(returnItem.alias->name, std::move(value));
    }
    inserter.NextRow();
  }

  return newTable;
}

void Demo::Process(const gql::ast::ForStatement& stmt,
                   ExecutionContext& context) {
  ContextStateChecker contextStateChecker(stmt, context);

  BindingTable result;
  for (auto& [colName, _] : context.workingTable.columns()) {
    result.AddColumn(colName);
  }
  result.AddColumn(stmt.alias.name);
  if (stmt.ordinalityOrOffset) {
    result.AddColumn(stmt.ordinalityOrOffset->var.name);
  }

  for (size_t i = 0; i < context.workingTable.size(); i++) {
    auto childContext = context.MakeAmended(i);
    auto source = Evaluate(stmt.source, childContext);
    BindingTable valuesTable;
    gql::ast::variant_switch(
        source.v, [&](const auto&) { ThrowInvalidValueTypeError(stmt.source); },
        [&](const BindingTableReference& tableValue) {
          auto inserter = valuesTable.CreateColumnInserter();
          for (size_t i = 0; i < tableValue->size(); i++) {
            inserter.Set(stmt.alias.name, tableValue->rowRecord(i));
            if (stmt.ordinalityOrOffset) {
              inserter.Set(
                  stmt.ordinalityOrOffset->var.name,
                  static_cast<int64_t>(
                      stmt.ordinalityOrOffset->isOrdinality ? (i + 1) : i));
            }
            inserter.NextRow();
          }
        },
        [&](const List& listValue) {
          auto inserter = valuesTable.CreateColumnInserter();
          int64_t idx = 0;
          for (auto& item : listValue) {
            inserter.Set(stmt.alias.name, *item);
            if (stmt.ordinalityOrOffset) {
              inserter.Set(
                  stmt.ordinalityOrOffset->var.name,
                  stmt.ordinalityOrOffset->isOrdinality ? (idx + 1) : idx);
              idx++;
            }
            inserter.NextRow();
          }
        });
    if (!valuesTable.columns().empty()) {
      result.AppendProduct(context.workingTable, i, valuesTable);
    }
  }
  context.workingTable = std::move(result);
}

}  // namespace gql_demo