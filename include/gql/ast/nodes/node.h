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

#pragma once

#include <limits>

namespace antlr4 {
class ParserRuleContext;
}

namespace gql::ast {

struct InputPosition {
  InputPosition() = default;
  InputPosition(size_t line, size_t col) : line(line), col(col) {}

  size_t line = std::numeric_limits<size_t>::max();
  size_t col = std::numeric_limits<size_t>::max();

  bool IsSet() const { return line != std::numeric_limits<size_t>::max(); }
  bool operator<(const InputPosition& other) const {
    return line < other.line || (line == other.line && col < other.col);
  }
  bool operator>(const InputPosition& other) const {
    return line > other.line || (line == other.line && col > other.col);
  }
  bool operator==(const InputPosition& other) const {
    return line == other.line && col == other.col;
  }
};

struct Node {
  // inputPosition() returns position of the node in the input stream if it was
  // created by parser with the only exception: if child class has MaybeNotSet()
  // method and it returns true then it position data may not be set. Reason is
  // that some nodes are optional in GQL grammar but in AST they may be always
  // present in the parent node to simplify user code. E.g. instead of using
  // optional vector, we just leave vector empty if there is no corresponding
  // node in the parse tree.
  const InputPosition& inputPosition() const { return inputPosition_; }
  void SetInputPosition(const InputPosition& inputPosition) {
    inputPosition_ = inputPosition;
  }

  bool operator==(const Node& other) const = delete;

#ifndef NDEBUG
  // Used only for debugging. Requires GQLParser.setBuildParseTree(true) and
  // live parser data.
  antlr4::ParserRuleContext* parserContext = nullptr;
#endif

 private:
  InputPosition inputPosition_{};
};

}  // namespace gql::ast
