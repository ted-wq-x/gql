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

#include <string>
#include <vector>

#include "gql/ast/nodes/path_patterns.h"

namespace gql_demo::helpers {

std::vector<gql::ast::PathPatternExpression> SplitPathUnionByVariable(
    const gql::ast::PathPatternExpression& pathPattern,
    const std::string& var);

const gql::ast::ElementPattern* FindVarInPathPatternTopLevel(
    const gql::ast::PathPatternExpression&,
    const std::string& var);

}  // namespace gql_demo::helpers