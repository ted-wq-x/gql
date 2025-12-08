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

#include <format>

#include "error.h"
#include "gql/ast/algorithm.h"
#include "gql/syntax_analyzer/aux_data.h"
#include "graph_pattern_planner.h"
#include "op_sequence.h"
#include "operators.h"
#include "timeout_watchdog.h"

namespace gql_demo {

SearchOpPtr CreateSaveMatchOp(
    const gql::ast::SimpleMatchStatement&,
    const std::unordered_set<std::string>& overlapFields,
    const std::vector<std::string>& fieldsToCopyToResult,
    const std::vector<std::string>& effectiveYield,
    BindingTable::Inserter&);

void InitializeSearchContextWithIncomingBindings(
    const gql::ast::SimpleMatchStatement& statement,
    Demo::ExecutionContext& executionContext,
    SearchContext& searchContext,
    const std::unordered_set<std::string>& overlapFields,
    bool* matchImpossible);

void Demo::Process(const gql::ast::SimpleMatchStatement& statement,
                   ExecutionContext& context) {
  ContextStateChecker contextStateChecker(statement, context);

  // Contains names that are both in working record or table and also are
  // defined as pattern variables.
  // These fields are removed from working record used in pattern matching.
  std::unordered_set<std::string> overlapFields;
  for (auto& var : statement.pattern.auxData->variables) {
    if (context.workingTable.columns().count(var.first) ||
        context.workingRecord.hasField(var.first)) {
      overlapFields.insert(var.first);
    }
  }

  // Result table contains all fields from working table that are not
  // overlapping with pattern variables.
  // Keeping these fields in the order they appear in the working table.
  std::vector<std::string> fieldsToCopyToResult;
  for (auto& col : context.workingTable.preferredColumnSequence()) {
    if (!overlapFields.count(col)) {
      fieldsToCopyToResult.push_back(col);
    }
  }

  std::vector<std::string> effectiveYield;
  if (statement.yield) {
    for (auto& item : *statement.yield) {
      if (!context.workingRecord.hasField(item.name)) {
        // 14.4 General Rule 1: "without the columns identified by IREDTFNS
        // [incoming working record]"
        effectiveYield.push_back(item.name);
      }
    }
  } else {
    for (auto& [var, varDef] : statement.pattern.auxData->variables) {
      if (!varDef.isTemp &&
          varDef.type != gql::GraphPatternVariableType::Subpath &&
          !context.workingRecord.hasField(var)) {
        effectiveYield.push_back(var);
      }
    }
  }

  BindingTable table;
  for (auto& col : fieldsToCopyToResult) {
    table.AddColumn(col, SetPreferredColumnSequence);
  }
  for (auto& col : effectiveYield) {
    table.AddColumn(col, SetPreferredColumnSequence);
  }
  auto inserter = table.CreateInserter();

  SearchOpPtr firstOp;
  {
    StoringOpSequence storingOpSequence(firstOp);
    LinearOpSequence opSequence(storingOpSequence);
    GraphPatternPlanner::CreateExecutionPlan(statement, opSequence,
                                             overlapFields);
    opSequence.Append(CreateSaveMatchOp(statement, overlapFields,
                                        fieldsToCopyToResult, effectiveYield,
                                        inserter));
  }

  SearchContext::EvaluateSearchConditionFunc evaluateSearchCondition =
      [&](const gql::ast::ValueExpression& expr, const WorkingRecord& record) {
        ExecutionContext tempContext;
        tempContext.workingRecord = record;
        auto value = Evaluate(expr, tempContext);
        if (auto boolPtr = std::get_if<bool>(&value.v)) {
          return *boolPtr;
        }
        ThrowInvalidValueTypeError(expr);
      };

  try {
    for (size_t i = 0; i < context.workingTable.size(); i++) {
      auto childContext = context.MakeAmended(i);
      SearchContext searchContext(graph, childContext.workingRecord,
                                  evaluateSearchCondition,
                                  statement.pattern.paths.size(),
                                  statement.pattern.matchMode &&
                                      *statement.pattern.matchMode ==
                                          gql::ast::MatchMode::DifferentEdges,
                                  watchdog_);

      for (size_t pathIdx = 0; pathIdx < statement.pattern.paths.size();
           pathIdx++) {
        auto& path = statement.pattern.paths[pathIdx];
        if (path.var) {
          searchContext.SetPathVariable(pathIdx, path.var->name);
        }
      }

      bool matchImpossible = false;
      InitializeSearchContextWithIncomingBindings(statement, childContext,
                                                  searchContext, overlapFields,
                                                  &matchImpossible);
      if (matchImpossible) {
        continue;
      }

      firstOp->Execute(searchContext);
    }
  } catch (TimeoutException& e) {
    e.SetCurrentTableSize(table.size());
    throw e;
  }

  context.workingTable = std::move(table);
}

SearchOpPtr CreateSaveMatchOp(
    const gql::ast::SimpleMatchStatement& statement,
    const std::unordered_set<std::string>& overlapFields,
    const std::vector<std::string>& fieldsToCopyToResult,
    const std::vector<std::string>& effectiveYield,
    BindingTable::Inserter& inserter) {
  return std::make_shared<SaveMatchOp>([&](const SearchContext& searchContext) {
    // We need to join path match with the working record. So first we
    // calculate the values for the pattern variables and only add the row
    // if it matches the working record on the overlapping fields.
    std::vector<Value> rowValues;
    rowValues.reserve(effectiveYield.size());
    bool mismatch = false;
    for (auto& var : effectiveYield) {
      auto& value = rowValues.emplace_back();
      auto& varDef = statement.pattern.auxData->variables.find(var)->second;
      switch (varDef.type) {
        case gql::GraphPatternVariableType::Node:
        case gql::GraphPatternVariableType::Edge: {
          auto bindingsIt = searchContext.currentBindings.find(var);
          assert(bindingsIt != searchContext.currentBindings.end());
          assert(bindingsIt->second.reference()->bindings.has_value());
          value = searchContext.GetElementBindingsValue(
              varDef.type, varDef.degreeOfExposure,
              *bindingsIt->second.reference()->bindings);
          break;
        }
        case gql::GraphPatternVariableType::Path:
          value.v = searchContext.pathReferenceValue(var);
          break;
        default:
          assert(false);
      }
      if (overlapFields.count(var)) {
        if (searchContext.workingRecord.fieldValue(var) != value) {
          mismatch = true;
          break;
        }
      }
    }
    if (mismatch) {
      return;
    }
    for (auto& col : fieldsToCopyToResult) {
      inserter.Set(col, searchContext.workingRecord.fieldValue(col));
    }
    for (size_t i = 0; i < effectiveYield.size(); i++) {
      inserter.Set(effectiveYield[i], rowValues[i]);
    }
    inserter.NextRow();
  });
}

// Set unconditional singleton graph element variables to the values in the
// incoming working record with the same name if there are any.
// It is done to optimize pattern matching. Overlapping fields from the
// working record and graph pattern variables are compared anyway after each
// match is acquired.
void InitializeSearchContextWithIncomingBindings(
    const gql::ast::SimpleMatchStatement& statement,
    Demo::ExecutionContext& executionContext,
    SearchContext& searchContext,
    const std::unordered_set<std::string>& overlapFields,
    bool* matchImpossible) {
  for (auto& [var, varDef] : statement.pattern.auxData->variables) {
    if (overlapFields.count(var) &&
        varDef.degreeOfExposure ==
            gql::VariableDegreeOfExposure::UnconditionalSingleton) {
      auto incomingVarBinding = executionContext.workingRecord.fieldValue(var);
      Graph::EdgeOrNodeId elementId;
      gql::ast::variant_switch(
          incomingVarBinding.v,
          [&](const auto&) {
            // data exception - values not comparable (22G04)
            throw GQLRuntimeError(
                statement.inputPosition(),
                std::format("not comparable values in field {0}", var));
          },
          [&](const Null&) {
            // Not sure, maybe it's syntax error, but anyway matching null to
            // conditional singleton is impossible.
            *matchImpossible = true;
          },
          [&](const NodeReference& value) {
            if (varDef.type != gql::GraphPatternVariableType::Node) {
              // data exception - values not comparable (22G04)
              throw GQLRuntimeError(
                  statement.inputPosition(),
                  std::format("not comparable values in field {0}", var));
            }
            elementId = value.nodeId;
          },
          [&](const EdgeReference& value) {
            if (varDef.type != gql::GraphPatternVariableType::Edge) {
              // data exception - values not comparable (22G04)
              throw GQLRuntimeError(
                  statement.inputPosition(),
                  std::format("not comparable values in field {0}", var));
            }
            elementId = value.edgeId;
          });
      if (*matchImpossible) {
        return;
      }
      auto& varBindings = searchContext.boundElementVariable(var);
      varBindings.push_back(elementId);
      searchContext.currentBindings[var].Bind({varBindings.begin(), 1}, true);
    }
  }
}

}  // namespace gql_demo