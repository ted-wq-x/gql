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

#include "variable_bindings_scope.h"

#include "graph_pattern_context.h"
#include "op_sequence.h"

namespace gql_demo {

GraphPatternPlanner::VariableBindingsLevelScope::VariableBindingsLevelScope(
    GraphPatternContext& context,
    const gql::syntax_analyzer::PathVariableReferenceScopeAuxData& auxData,
    LinearOpSequence& opSequence)
    : context_(context), auxData_(auxData), opSequence_(opSequence) {
  if (auxData.declaredVariables.empty()) {
    return;
  }

  auto bindingsLevelOp = std::make_shared<VariableBindingsLevelOp>();
  for (auto& [var, _] : auxData.declaredVariables) {
    context.variables[var].PushLevel();
    bindingsLevelOp->variables.push_back(var);
  }
  opSequence.Append(bindingsLevelOp);
  exitOp_ = bindingsLevelOp->CreateExitOp();
}

GraphPatternPlanner::VariableBindingsLevelScope::~VariableBindingsLevelScope() {
  if (exitOp_) {
    opSequence_.Append(std::move(exitOp_));
  }

  for (auto& [var, _] : auxData_.declaredVariables) {
    context_.variables[var].PopLevel();
  }
}

GraphPatternPlanner::VariableBindingsScope::VariableBindingsScope(
    PathPatternContext& context,
    const gql::syntax_analyzer::PathVariableReferenceScopeAuxData& auxData,
    LinearOpSequence& opSequence)
    : context_(context),
      auxData_(auxData),
      opSequence_(opSequence),
      waitingForStartElement_(context.startFromElement) {
  if (auxData.declaredVariables.empty()) {
    return;
  }

  enterOp_ = std::make_shared<VariableBindingsOp>();
  for (auto& [var, varDef] : auxData.declaredVariables) {
    enterOp_->variables.emplace_back(
        var, varDef.degreeOfExposure ==
                 gql::VariableDegreeOfExposure::UnconditionalSingleton);
  }
  opSequence.Append(enterOp_);
}

GraphPatternPlanner::VariableBindingsScope::~VariableBindingsScope() {
  if (auxData_.declaredVariables.empty()) {
    return;
  }

  if (waitingForStartElement_ != !!context_.startFromElement) {
    assert(auxData_.declaredVariables.size() == 1 &&
           auxData_.declaredVariables.begin()->second.degreeOfExposure ==
               gql::VariableDegreeOfExposure::UnconditionalSingleton);
  }

  if (waitingForStartElement_ &&
      (!!context_.startFromElement || context_.reverseDirection)) {
    opSequence_.Remove(enterOp_);
    return;
  }

  opSequence_.Append(enterOp_->CreateExitOp());
  // After processing of the factor, mark variables as bound at the
  // current level.
  for (auto& outVar : auxData_.declaredVariables) {
    auto& var = context_.graph.variables[outVar.first];
    var.Bind(outVar.second.degreeOfExposure ==
             gql::VariableDegreeOfExposure::UnconditionalSingleton);
  }
  MaybeAppendCheckCondition(context_.graph, opSequence_);
}

}  // namespace gql_demo