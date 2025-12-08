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

#include "path_mode_scope.h"

#include "graph_pattern_context.h"

namespace gql_demo::helpers {

PathModeScope::PathModeScope(gql::ast::PathMode pathMode,
                             LinearOpSequence& opSequence,
                             PathPatternContext& patternContext)
    : opSequence_(opSequence),
      patternContext_(patternContext),
      waitingForStartElement_(patternContext.startFromElement),
      oldPathModeUniqueNodes_(patternContext.pathModeUniqueNodes),
      oldPathModeUniqueEdges_(patternContext.pathModeUniqueEdges) {
  switch (pathMode) {
    case gql::ast::PathMode::WALK:
      break;
    case gql::ast::PathMode::TRAIL:
      if (!patternContext.pathModeUniqueEdges) {
        auto enterOp =
            std::make_shared<PathModeUniqueEdgesOp>(/*startOp*/ true);
        enterOpState_ = enterOp.get();
        auto endOp = std::make_shared<PathModeUniqueEdgesOp>(/*startOp*/ false);
        exitOp_ = endOp;
        exitOpState_ = endOp.get();
        opSequence.Append(std::move(enterOp));
        patternContext.pathModeUniqueEdges = true;
      }
      break;
    case gql::ast::PathMode::ACYCLIC:
      if (!patternContext.pathModeUniqueNodes) {
        auto enterOp =
            std::make_shared<PathModeUniqueNodesOp>(/*startOp*/ true);
        enterOpState_ = enterOp.get();
        auto endOp = std::make_shared<PathModeUniqueNodesOp>(/*startOp*/ false);
        exitOp_ = endOp;
        exitOpState_ = endOp.get();
        opSequence.Append(std::move(enterOp));
        patternContext.pathModeUniqueNodes = true;
      }
      break;
    case gql::ast::PathMode::SIMPLE:
      // Feature disabled
      assert(false);
      break;
  }
}

PathModeScope::PathModeScope(
    const std::optional<gql::ast::PathPatternPrefix>& pathPatternPrefix,
    LinearOpSequence& opSequence,
    PathPatternContext& patternContext)
    : PathModeScope(pathPatternPrefix ? pathPatternPrefix->mode
                                      : gql::ast::PathMode::WALK,
                    opSequence,
                    patternContext) {}

PathModeScope::~PathModeScope() {
  patternContext_.pathModeUniqueNodes = oldPathModeUniqueNodes_;
  patternContext_.pathModeUniqueEdges = oldPathModeUniqueEdges_;

  if (exitOp_) {
    if (waitingForStartElement_ && !patternContext_.startFromElement) {
      // If path processing started from an element inside the path mode scope
      // then on the way forward we need to save the state when exiting the
      // scope and restore it when entering the scope on the way back.
      if (patternContext_.reverseDirection) {
        enterOpState_->SetRestoreStateOnEnter();
      } else {
        exitOpState_->SetSaveStateOnExit();
      }
    }

    opSequence_.Append(std::move(exitOp_));
  }
}

}  // namespace gql_demo::helpers