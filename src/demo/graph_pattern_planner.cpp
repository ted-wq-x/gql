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

#include "graph_pattern_planner.h"

#include <list>
#include <numeric>
#include <ranges>

#include "gql/ast/algorithm.h"
#include "gql/syntax_analyzer/aux_data.h"
#include "graph_pattern_context.h"
#include "helpers/directional_iterator.h"
#include "helpers/path_mode_scope.h"
#include "helpers/scope_exit.h"
#include "helpers/split_path_union_by_variable.h"
#include "helpers/variable_bindings_scope.h"
#include "op_sequence.h"
#include "operators.h"

namespace gql_demo {

void GraphPatternPlanner::CreateExecutionPlan(
    const gql::ast::SimpleMatchStatement& statement,
    LinearOpSequence& opSequence,
    const std::unordered_set<std::string>& overlapFields) {
  GraphPatternContext patternContext;

  // If uncondition singleton element variable has corresponding field in the
  // incoming working record, we consider it bound from the start.
  for (auto& [var, varDef] : statement.pattern.auxData->variables) {
    if (overlapFields.count(var) &&
        varDef.degreeOfExposure ==
            gql::VariableDegreeOfExposure::UnconditionalSingleton) {
      patternContext.boundJoinableVariables.top().insert(var);
    }
  }

  if (statement.pattern.where) {
    AddCheckCondition(patternContext, opSequence,
                      statement.pattern.where->condition,
                      *statement.pattern.where->auxData);
  }

  std::list<size_t> pathsToProcess(statement.pattern.paths.size());
  std::iota(pathsToProcess.begin(), pathsToProcess.end(), 0);
  while (!pathsToProcess.empty()) {
    // 1. Search for a path pattern that can be joined on one of already bound
    // variables.
    const gql::ast::PathPattern* currentPathPattern = nullptr;
    for (auto pathIdx = pathsToProcess.begin(); pathIdx != pathsToProcess.end();
         pathIdx++) {
      auto& pathPattern = statement.pattern.paths[*pathIdx];
      auto& pathVariables = pathPattern.auxData->joinableVariables;
      auto joinVarIt = std::ranges::find_if(
          patternContext.boundJoinableVariables.top(),
          [&](const auto& var) { return pathVariables.count(var); });
      if (joinVarIt == patternContext.boundJoinableVariables.top().end()) {
        continue;
      }

      opSequence.Append(std::make_shared<SetCurrentPathOp>(*pathIdx));
      ProcessPathJoinedOnVar(pathPattern, *joinVarIt, patternContext,
                             opSequence);
      pathsToProcess.erase(pathIdx);
      currentPathPattern = &pathPattern;
      break;
    }
    // 2. If no such path pattern is found, pick an arbitrary unprocessed
    // path.
    if (!currentPathPattern) {
      auto& pathPattern = statement.pattern.paths[pathsToProcess.front()];
      opSequence.Append(
          std::make_shared<SetCurrentPathOp>(pathsToProcess.front()));
      opSequence.Append(std::make_shared<ScanAllNodesOp>());
      PathPatternContext pathPatternContext(patternContext);
      helpers::PathModeScope pathModeScope(pathPattern.prefix, opSequence,
                                           pathPatternContext);
      Process(pathPattern.expr, pathPatternContext, opSequence);
      pathsToProcess.pop_front();
      currentPathPattern = &pathPattern;
    }

    auto& pathVariables = currentPathPattern->auxData->joinableVariables;
    patternContext.boundJoinableVariables.top().insert(pathVariables.begin(),
                                                       pathVariables.end());

    if (currentPathPattern->var) {
      patternContext.variables[currentPathPattern->var->name].Bind(
          /*isSingleton*/ true);
      MaybeAppendCheckCondition(patternContext, opSequence);
    }
  }
}

void GraphPatternPlanner::ProcessPathJoinedOnVar(
    const gql::ast::PathPattern& pathPattern,
    const std::string& var,
    GraphPatternContext& patternContext,
    LinearOpSequence& opSequence) {
  auto processPath = [&](const gql::ast::ElementPattern* pathElement,
                         GraphPatternContext& patternContext) {
    PathPatternContext pathPatternContext(patternContext);
    helpers::PathModeScope pathModeScope(pathPattern.prefix, opSequence,
                                         pathPatternContext);
    // 1. Process starting from the element in forward direction
    pathPatternContext.startFromElement = pathElement;
    Process(pathPattern.expr, pathPatternContext, opSequence);
    assert(pathPatternContext.startFromElement == nullptr);
    opSequence.Append(std::make_shared<SetCurrentPathReverseDirectionOp>());
    // 2. Process starting from the element in reverse direction
    pathPatternContext.startFromElement = pathElement;
    pathPatternContext.reverseDirection = true;
    Process(pathPattern.expr, pathPatternContext, opSequence);
    assert(pathPatternContext.startFromElement == nullptr);
  };

  if (auto* pathElement =
          helpers::FindVarInPathPatternTopLevel(pathPattern.expr, var)) {
    processPath(pathElement, patternContext);
  } else {
    // We can't join on the variable that is inside union, so we split
    // path pattern with union around target variable into the union of path
    // patterns and process each resulting path pattern.
    auto originalGraphPatternContext = patternContext;
    std::optional<GraphPatternContext> outputPatternContext;
    for (auto& replacementPath :
         helpers::SplitPathUnionByVariable(pathPattern.expr, var)) {
      auto* pathElement =
          helpers::FindVarInPathPatternTopLevel(replacementPath, var);
      assert(pathElement);

      // Each replacement path pattern processing is started with the original
      // state of GraphPatternContext and we expect resulting states to be the
      // same.
      auto graphPatternContext = originalGraphPatternContext;

      processPath(pathElement, graphPatternContext);

      if (outputPatternContext) {
        assert(*outputPatternContext == graphPatternContext);
      } else {
        outputPatternContext = std::move(graphPatternContext);
      }
    }
    patternContext = *outputPatternContext;
  }
}

void GraphPatternPlanner::Process(
    const gql::ast::PathPatternExpression& pathPattern,
    PathPatternContext& patternContext,
    LinearOpSequence& pathExprOpSequence) {
  if (patternContext.startFromElement && pathPattern.terms.size() > 1) {
    // Start element can't be inside union.
    return;
  }

  VariableBindingsScope variableScopeUnion(patternContext, *pathPattern.auxData,
                                           pathExprOpSequence);

  ForkOpSequence unionForkOpSequence(pathExprOpSequence);
  for (auto& term : pathPattern.terms) {
    LinearOpSequence termOpSequence(unionForkOpSequence);

    VariableBindingsLevelScope variableScopeLevelUnion(
        patternContext.graph, *pathPattern.auxData, termOpSequence);

    for (helpers::DirectionalIterator factorIdx{
             term, patternContext.reverseDirection};
         factorIdx; factorIdx++) {
      auto& factor = term[*factorIdx];
      helpers::ScopeExit factorScopeExit;

      if (patternContext.startFromElement &&
          !std::holds_alternative<gql::ast::PathFactor::NoQuantifier>(
              factor.quantifier)) {
        // Start element can't be inside quantifier.
        continue;
      }

      VariableBindingsScope variableScopeFactor(patternContext, *factor.auxData,
                                                termOpSequence);

      if (!std::holds_alternative<gql::ast::PathFactor::NoQuantifier>(
              factor.quantifier)) {
        // Patterns inside conditional or quantified factors can join to outer
        // singleton variables, but not other way around.
        patternContext.graph.boundJoinableVariables.push(
            patternContext.graph.boundJoinableVariables.top());

        factorScopeExit.Add([&]() {
          patternContext.graph.boundJoinableVariables.pop();
          for (auto& factorVar : factor.auxData->declaredVariables) {
            // If union exposes variable as unconditional singleton, it can be
            // joined to in the outer context.
            if (factorVar.second.degreeOfExposure ==
                gql::VariableDegreeOfExposure::UnconditionalSingleton) {
              patternContext.graph.boundJoinableVariables.top().insert(
                  factorVar.first);
            }
          }
        });
      }

      std::optional<ForkOpSequence> optionalForkOpSequence;
      std::optional<QuantifiedOpSequence> optionalQuantifiedOpSequence;
      std::optional<LinearOpSequence> optionalLinearOpSequence;
      gql::ast::variant_switch(
          factor.quantifier, [&](const gql::ast::PathFactor::NoQuantifier&) {},
          [&](const gql::ast::PathFactor::Optional&) {
            optionalForkOpSequence.emplace(termOpSequence);
            optionalForkOpSequence->InsertShortcut();
            optionalLinearOpSequence.emplace(*optionalForkOpSequence);
          },
          [&](const gql::ast::GraphPatternQuantifier& value) {
            auto op = std::make_shared<QuantifiedOp>(value.lower, value.upper);
            termOpSequence.Append(op);
            optionalQuantifiedOpSequence.emplace(op);
            optionalLinearOpSequence.emplace(*optionalQuantifiedOpSequence);
          });

      LinearOpSequence& factorOpSequence =
          optionalLinearOpSequence ? *optionalLinearOpSequence : termOpSequence;

      VariableBindingsLevelScope variableScopeLevelFactor(
          patternContext.graph, *factor.auxData, factorOpSequence);

      gql::ast::variant_switch(
          factor.pattern,
          [&](const gql::ast::ElementPattern& value) {
            if (patternContext.startFromElement) {
              if (&value != patternContext.startFromElement) {
                // Skip until we reach the start element.
                return;
              }
              patternContext.startFromElement = nullptr;
              if (patternContext.reverseDirection) {
                // Process starting element itself on left-to-right direction
                // only. On the right-to-left pass start with the next one.
                return;
              }
            }

            gql::ast::variant_switch(
                value,
                [&](const gql::ast::NodePattern& value) {
                  if (value.filler.var) {
                    auto& var = value.filler.var->name;
                    if (!patternContext.graph.boundJoinableVariables.top()
                             .emplace(var)
                             .second) {
                      assert(!value.filler.var->isTemp);
                      // Node is joinable in current level of path expression
                      // and already bound, join to it.
                      factorOpSequence.Append(
                          std::make_shared<JoinNodeOp>(var));
                    } else {
                      factorOpSequence.Append(std::make_shared<BindElementOp>(
                          var, /*isNode*/ true));
                    }
                  }
                  factorOpSequence.Append(
                      std::make_shared<ReferenceCurrentNodeOp>());
                  // TODO: If joining to matched node move checks earlier.
                  if (value.filler.labelExpr) {
                    factorOpSequence.Append(std::make_shared<CheckNodeLabelsOp>(
                        *value.filler.labelExpr));
                  }
                  if (value.filler.predicate) {
                    assert(false);
                  }
                },
                [&](const gql::ast::EdgePattern& value) {
                  auto op = std::make_shared<ExpandEdgeOp>(value.direction);
                  factorOpSequence.Append(op);
                  if (value.filler.var) {
                    auto& var = value.filler.var->name;
                    if (!patternContext.graph.boundJoinableVariables.top()
                             .emplace(var)
                             .second) {
                      assert(!value.filler.var->isTemp);
                      // Edge is joinable in current level of path expression
                      // and already bound, join to it.
                      op->JoinEdge(var);
                    } else {
                      factorOpSequence.Append(std::make_shared<BindElementOp>(
                          var, /*isNode*/ false));
                    }
                  }
                  if (value.filler.labelExpr) {
                    op->SetLabelExpression(*value.filler.labelExpr);
                  }
                });
          },
          [&](const gql::ast::ParenthesizedPathPatternExpressionPtr& value) {
            if (value->where) {
              // TODO: What about starting from the middle of the pattern?
              AddCheckCondition(patternContext.graph, termOpSequence,
                                value->where->condition,
                                *value->where->auxData);
            }

            helpers::PathModeScope pathModeScope(
                value->pathMode, factorOpSequence, patternContext);
            Process(value->pattern, patternContext, factorOpSequence);
          },
          [&](const gql::ast::SimplifiedPathPatternExpression&) {
            assert(false);
          });
    }
  }
}

// Immediately adds closure operator to capture already bound or future element
// variable matches and creates pending condition to be checked later when all
// referenced variables are bound.
void GraphPatternPlanner::AddCheckCondition(
    GraphPatternContext& patternContext,
    LinearOpSequence& opSequence,
    const gql::ast::SearchCondition& cond,
    const gql::syntax_analyzer::GraphPatternWhereClauseAuxData& auxData) {
  auto condIndex = patternContext.nextConditionIndex++;
  auto conditionClosureOp = std::make_shared<AddConditionClosureOp>(condIndex);
  opSequence.Append(conditionClosureOp);

  GraphPatternContext::PendingCondition condition;
  condition.op = std::make_shared<CheckConditionOp>(*cond, auxData, condIndex);
  for (auto& [var, _] : auxData.referencedVariables) {
    condition.referencedVars.push_back(
        patternContext.variables[var].reference());
    conditionClosureOp->variables.push_back(var);
  }

  if (condition.isReadyToEvaluate()) {
    opSequence.Append(condition.op);
  } else {
    patternContext.pendingConditions.push_back(std::move(condition));
  }
}

// Called after binding new variables to check if any pending condition
// can be evaluated now.
void GraphPatternPlanner::MaybeAppendCheckCondition(
    GraphPatternContext& patternContext,
    LinearOpSequence& opSequence) {
  for (auto condIt = patternContext.pendingConditions.begin();
       condIt != patternContext.pendingConditions.end();) {
    if (condIt->isReadyToEvaluate()) {
      opSequence.Append(condIt->op);
      condIt = patternContext.pendingConditions.erase(condIt);
    } else {
      ++condIt;
    }
  }
}

}  // namespace gql_demo