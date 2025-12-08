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

#include "gql/ast/nodes/path_patterns.h"
#include "op_sequence.h"
#include "operators.h"

namespace gql_demo {

struct PathPatternContext;

namespace helpers {

class PathModeScope {
 public:
  PathModeScope(gql::ast::PathMode pathMode,
                LinearOpSequence& opSequence,
                PathPatternContext& patternContext);
  PathModeScope(
      const std::optional<gql::ast::PathPatternPrefix>& pathPatternPrefix,
      LinearOpSequence& opSequence,
      PathPatternContext& patternContext);
  ~PathModeScope();

 private:
  LinearOpSequence& opSequence_;
  PathPatternContext& patternContext_;
  const bool waitingForStartElement_;
  const bool oldPathModeUniqueNodes_;
  const bool oldPathModeUniqueEdges_;
  std::shared_ptr<SearchOp> exitOp_;
  PathModeOpState* enterOpState_ = nullptr;
  PathModeOpState* exitOpState_ = nullptr;
};

}  // namespace helpers

}  // namespace gql_demo