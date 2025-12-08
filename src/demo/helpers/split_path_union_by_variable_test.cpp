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

#include <gtest/gtest.h>

#include <unordered_set>

#include "gql/ast/print.h"
#include "gql/parser/parser.h"
#include "split_path_union_by_variable.h"

struct RewriteTestParam {
  std::string original;
  std::unordered_set<std::string> expected;
};

class SplitPathUnionByVariable
    : public testing::TestWithParam<RewriteTestParam> {};

TEST_P(SplitPathUnionByVariable, Check) {
  gql::ast::GQLProgram program;
  try {
    program = gql::parser::ParseProgram(GetParam().original.c_str());
  } catch (const std::runtime_error& e) {
    GTEST_FAIL() << e.what();
  }

  auto* pathExpr =
      gql::ast::FindFirstNodeOfType<gql::ast::PathPatternExpression>(program);
  ASSERT_TRUE(pathExpr);

  auto splitResult =
      gql_demo::helpers::SplitPathUnionByVariable(*pathExpr, "var");
  auto expected = GetParam().expected;
  EXPECT_EQ(splitResult.size(), expected.size());
  for (auto& expr : splitResult) {
    std::string printed = gql::ast::PrintTree(expr);
    EXPECT_TRUE(expected.erase(printed) == 1) << printed;
  }
}

INSTANTIATE_TEST_SUITE_P(
    ,
    SplitPathUnionByVariable,
    testing::Values(
        RewriteTestParam{"MATCH (a)-(var) | (var)-(b)",
                         {"(a)-(var)", "(var)-(b)"}},
        RewriteTestParam{
            "MATCH (a)-((c)-((var)-(e) | (f)-(var)))-(b)",
            {"(a)- ((c)- ((var)-(e))) -(b)", "(a)- ((c)- ((f)-(var))) -(b)"}},
        RewriteTestParam{
            "MATCH (a)-((c)-((var)-(e) | (f)-((var)-(g) | (e)-(var))))-(b)",
            {"(a)- ((c)- ((var)-(e))) -(b)",
             "(a)- ((c)- ((f)- ((var)-(g)))) -(b)",
             "(a)- ((c)- ((f)- ((e)-(var)))) -(b)"}}));
