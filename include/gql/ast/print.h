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

#include "gql/ast/print/common.h"
#include "gql/ast/print/expression.h"
#include "gql/ast/print/generic.h"
#include "gql/ast/print/graph_types.h"
#include "gql/ast/print/output_stream.h"
#include "gql/ast/print/path_patterns.h"
#include "gql/ast/print/references.h"
#include "gql/ast/print/statements.h"
#include "gql/ast/print/types.h"
#include "gql/ast/print/pgq.h"

namespace gql::ast {

template <typename NodeType>
std::string PrintTree(NodeType& node) {
  print::OutputStream os;
  os << node;
  return os.str();
}

}  // namespace gql::ast
