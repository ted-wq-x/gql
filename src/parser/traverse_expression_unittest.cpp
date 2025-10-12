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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "gql/ast/algorithm.h"
#include "gql/parser/parser.h"

namespace {

struct VariableReferences {
  MOCK_METHOD(void, OnReference, (std::string));
};

}  // namespace

TEST(TraverseExpression, VariableReferences) {
  const char query[] = R"(
SESSION SET VALUE $PARAM1 = 1 + 3 * A / B.prop1
)";
  gql::ast::GQLProgram program = gql::parser::ParseProgram(query);
  const auto* expr =
      gql::ast::FindFirstNodeOfType<gql::ast::ValueExpression>(program);
  ASSERT_TRUE(expr);

  VariableReferences variableReferences;
  EXPECT_CALL(variableReferences, OnReference("A"));
  EXPECT_CALL(variableReferences, OnReference("B"));
  gql::ast::ForEachNodeInTree(
      *expr, gql::ast::overloaded(
                 [](auto*) { return gql::ast::VisitorResult::kContinue; },
                 [&variableReferences](
                     const gql::ast::BindingVariableReference* node) {
                   variableReferences.OnReference(node->name);
                   return gql::ast::VisitorResult::kContinue;
                 }));
}