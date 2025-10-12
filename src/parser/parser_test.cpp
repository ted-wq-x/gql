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
#include "gql/ast/print.h"
#include "gql/parser/parser.h"

namespace {

struct LabelsVisitor {
  LabelsVisitor() = default;
  LabelsVisitor(LabelsVisitor&&) = delete;
  LabelsVisitor(const LabelsVisitor&) = delete;
  LabelsVisitor& operator=(LabelsVisitor&&) = delete;
  LabelsVisitor& operator=(const LabelsVisitor&) = delete;

  auto operator()(const gql::ast::LabelSetSpecification&) {
    return gql::ast::VisitorResult::kContinue;
  }
};

struct NodePositions {
  MOCK_METHOD(void, OnNode, (gql::ast::InputPosition));
};

}  // namespace

TEST(ParserTest, ParseProgram) {
  const char query[] = R"(
MATCH (p:Person)-[r:IS_FRIENDS_WITH]->(friend:Person)
WHERE EXISTS (MATCH (p)-[:WORKS_FOR]->(:Company {name: "GQL, Inc."}))
RETURN p, r, friend
)";

  gql::ast::GQLProgram program = gql::parser::ParseProgram(query);
  EXPECT_TRUE(
      gql::ast::FindFirstNodeOfType<gql::ast::SimpleMatchStatement>(program));

  const gql::ast::GQLProgram programConstCopy = program;
  EXPECT_TRUE(gql::ast::FindFirstNodeOfType<gql::ast::SimpleMatchStatement>(
      programConstCopy));

  NodePositions simpleMatchStatementNodes;
  EXPECT_CALL(simpleMatchStatementNodes, OnNode(gql::ast::InputPosition{2, 0}));
  EXPECT_CALL(simpleMatchStatementNodes,
              OnNode(gql::ast::InputPosition{3, 14}));
  gql::ast::ForEachNodeOfType<gql::ast::SimpleMatchStatement>(
      program, [&](const gql::ast::SimpleMatchStatement& node) {
        simpleMatchStatementNodes.OnNode(node.inputPosition());
        return gql::ast::VisitorResult::kContinue;
      });

  {
    int count = 0;
    gql::ast::ForEachNodeInTree(program, [&count](auto*) {
      count++;
      return gql::ast::VisitorResult::kContinue;
    });
    EXPECT_GT(count, 100);
  }
  {
    int count = 0;
    gql::ast::ForEachNodeInTree(programConstCopy, [&count](auto*) {
      count++;
      return gql::ast::VisitorResult::kContinue;
    });
    EXPECT_GT(count, 100);
  }

  EXPECT_EQ(
      gql::ast::PrintTree(program),
      R"(MATCH (p :Person)-[r :IS_FRIENDS_WITH]->(friend :Person) WHERE EXISTS {MATCH (p)-[:WORKS_FOR]->(:Company {name: "GQL, Inc."})} RETURN p, r, friend)");

  gql::ast::ForEachNodeOfType<gql::ast::LabelSetSpecification>(program,
                                                               LabelsVisitor());

  LabelsVisitor labelsVisitor;
  gql::ast::ForEachNodeOfType<gql::ast::LabelSetSpecification>(program,
                                                               labelsVisitor);
  gql::ast::ForEachNodeOfType<gql::ast::LabelSetSpecification>(programConstCopy,
                                                               labelsVisitor);
}

TEST(ParserTest, ParseError) {
  const char query[] = "sxjhjskxh jhxhsajx ja sjkxhsa";

  EXPECT_THROW(
      { gql::ast::GQLProgram program = gql::parser::ParseProgram(query); },
      gql::ParserError);
}

TEST(ASTTest, StaticCheck) {
  gql::ast::GQLProgram value;
  gql::ast::GQLProgram value2 = value;
  gql::ast::GQLProgram moved(std::move(value2));
  moved = std::move(value);
  gql::ast::GQLProgram valueCopied;
  valueCopied = moved;
  const gql::ast::GQLProgram constCopy = valueCopied;
  const gql::ast::GQLProgram constCopy2 = constCopy;
}
