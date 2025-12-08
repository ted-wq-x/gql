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

#include "error.h"

namespace gql_demo {

GQLRuntimeError::GQLRuntimeError(gql::ast::InputPosition pos,
                                 const std::string& msg)
    : runtime_error(msg), inputPosition_(pos), msg_(msg) {}

[[noreturn]] void ThrowInvalidValueTypeError(const gql::ast::Node& node) {
  throw GQLRuntimeError(node.inputPosition(), "invalid value type");
}

[[noreturn]] void ThrowInvalidReferenceError(const gql::ast::Node& node) {
  throw GQLRuntimeError(node.inputPosition(),
                        "reference value, referent deleted");
}

}  // namespace gql_demo