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
#include <filesystem>
#include <fstream>
#include <regex>
#include <type_traits>

#include "gql/ast/algorithm.h"
#include "gql/ast/ast.h"
#include "gql/ast/print.h"
#include "gql/parser/parser.h"
#include "input_positions_test.h"

class GQLFileTest : public testing::TestWithParam<std::filesystem::path> {
 public:
  static void TearDownTestSuite() { parserCache_.reset(); }

 protected:
  static std::unique_ptr<gql::parser::ParserCache> parserCache_;
  static int cacheUseCount_;
};

std::unique_ptr<gql::parser::ParserCache> GQLFileTest::parserCache_;
int GQLFileTest::cacheUseCount_ = 0;

TEST_P(GQLFileTest, Parse) {
  std::ifstream file(GetParam(), std::ios::binary);
  std::string query(std::istreambuf_iterator<char>(file), {});

  if (!parserCache_ || cacheUseCount_ > 100) {
    parserCache_ = std::make_unique<gql::parser::ParserCache>();
    cacheUseCount_ = 0;
  }
  cacheUseCount_++;

  gql::ast::GQLProgram program;
  try {
    program = gql::parser::ParseProgram(query.c_str(), {}, parserCache_.get());
  } catch (const std::runtime_error& e) {
    GTEST_FAIL() << e.what() << "\nparsing loaded query: " << query;
  }

  // Check that there are no unitialized pointers in the tree.
  int count = 0;
  gql::ast::ForEachNodeInTree(program, [&count](auto*) {
    count++;
    return gql::ast::VisitorResult::kContinue;
  });
  EXPECT_GT(count, 0);

  gql::ast::CheckInputPositions(program);

  const auto print1 = gql::ast::PrintTree(program);
  gql::ast::GQLProgram program1;
  try {
    program1 =
        gql::parser::ParseProgram(print1.c_str(), {}, parserCache_.get());
  } catch (const std::runtime_error& e) {
    GTEST_FAIL() << e.what() << "\nparsing printed query: " << print1;
  }

  const auto print2 = gql::ast::PrintTree(program1);

  EXPECT_EQ(print1, print2);
}

std::string GetTestName(
    const testing::TestParamInfo<std::filesystem::path>& info) {
  std::regex unsupportedSymbols("[^a-zA-Z0-9_]");
  return std::regex_replace(info.param.filename().string(), unsupportedSymbols,
                            "_");
}

std::vector<std::filesystem::path> GetTestFiles() {
  std::vector<std::filesystem::path> files;

  constexpr const char kEnvVarName[] = "GQL_TEST_QUERY_DIR";
  const char* path = std::getenv(kEnvVarName);
  if (!path) {
    return {};
  }

  const std::filesystem::path dir(path);
  for (auto const& dir_entry : std::filesystem::directory_iterator{dir})
    if (dir_entry.is_regular_file()) {
      files.push_back(dir_entry.path());
    }
  return files;
}

INSTANTIATE_TEST_SUITE_P(,
                         GQLFileTest,
                         testing::ValuesIn(GetTestFiles()),
                         GetTestName);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(GQLFileTest);