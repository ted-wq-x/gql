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

#include "check_label_expression.h"

#include "gql/ast/algorithm.h"

namespace gql_demo::helpers {

bool CheckLabelExpression(const gql::ast::LabelExpression& labelExpression,
                          const Graph& graph,
                          const Graph::LabelSet& labels) {
  return gql::ast::variant_switch(
      labelExpression.option,
      [&](const gql::ast::LabelExpression::Negation& value) {
        return !CheckLabelExpression(*value.expr, graph, labels);
      },
      [&](const gql::ast::LabelExpression::Logical& value) {
        std::optional<bool> res;
        for (auto& term : value.terms) {
          bool termResult = CheckLabelExpression(*term, graph, labels);
          if (!res) {
            res = termResult;
          } else {
            if (value.op == gql::ast::LabelExpression::Logical::Op::And) {
              *res &= termResult;
            } else {
              *res |= termResult;
            }
          }
        }
        return *res;
      },
      [&](const gql::ast::LabelName& value) {
        return labels.count(value.name) > 0;
      },
      [&](const gql::ast::LabelExpression::Wildcard&) {
        return !labels.empty();
      });
}

}  // namespace gql_demo::helpers