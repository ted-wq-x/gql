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

#include "split_path_union_by_variable.h"

#include <cassert>

#include "gql/ast/algorithm.h"
#include "gql/ast/ast.h"

namespace gql_demo::helpers {

static std::pair<bool, std::vector<gql::ast::PathPatternExpression>>
SplitPathUnionByVariableImpl(const gql::ast::PathPatternExpression& pathPattern,
                             const std::string& var) {
  std::vector<gql::ast::PathPatternExpression> result;
  bool resultVarFound = false;
  for (auto& term : pathPattern.terms) {
    for (auto factorIt = term.begin(); factorIt != term.end(); factorIt++) {
      auto& factor = *factorIt;
      if (!std::holds_alternative<gql::ast::PathFactor::NoQuantifier>(
              factor.quantifier)) {
        continue;
      }

      bool varFound = false;
      bool addedToResult = false;
      gql::ast::variant_switch(
          factor.pattern,
          [&](const gql::ast::ElementPattern& value) {
            gql::ast::variant_switch(value, [&](const auto& value) {
              if (value.filler.var && value.filler.var->name == var) {
                varFound = true;
              }
            });
          },
          [&](const gql::ast::ParenthesizedPathPatternExpressionPtr& value) {
            auto res = SplitPathUnionByVariableImpl(value->pattern, var);
            if (res.first) {
              varFound = true;
              if (res.second.size() > 1) {
                for (auto& primaryReplacement : res.second) {
                  result.emplace_back().terms.push_back(term);
                  auto& newPrimary =
                      result.back().terms.back()[factorIt - term.begin()];
                  std::get<gql::ast::ParenthesizedPathPatternExpressionPtr>(
                      newPrimary.pattern)
                      ->pattern = primaryReplacement;
                }
                addedToResult = true;
              }
            }
          },
          [&](const gql::ast::SimplifiedPathPatternExpression&) {
            assert(false);
          });
      if (varFound) {
        resultVarFound = true;
        if (!addedToResult && pathPattern.terms.size() > 1) {
          result.emplace_back().terms.push_back(term);
        }
        break;
      }
    }
  }
  if (resultVarFound) {
    assert(result.size() != 1);
  }
  return {resultVarFound, result};
}

std::vector<gql::ast::PathPatternExpression> SplitPathUnionByVariable(
    const gql::ast::PathPatternExpression& pathPattern,
    const std::string& var) {
  return SplitPathUnionByVariableImpl(pathPattern, var).second;
}

const gql::ast::ElementPattern* FindVarInPathPatternTopLevel(
    const gql::ast::PathPatternExpression& pathPattern,
    const std::string& var) {
  if (pathPattern.terms.size() != 1) {
    return nullptr;
  }

  const gql::ast::ElementPattern* result = nullptr;
  auto& term = pathPattern.terms.front();
  for (auto& factor : term) {
    if (!std::holds_alternative<gql::ast::PathFactor::NoQuantifier>(
            factor.quantifier)) {
      continue;
    }

    gql::ast::variant_switch(
        factor.pattern,
        [&](const gql::ast::ElementPattern& element) {
          gql::ast::variant_switch(element, [&](const auto& value) {
            if (value.filler.var && value.filler.var->name == var) {
              result = &element;
            }
          });
        },
        [&](const gql::ast::ParenthesizedPathPatternExpressionPtr& value) {
          result = FindVarInPathPatternTopLevel(value->pattern, var);
        },
        [&](const gql::ast::SimplifiedPathPatternExpression&) {
          assert(false);
        });
    if (result) {
      return result;
    }
  }
  return result;
}

}  // namespace gql_demo::helpers