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

#include <stdexcept>

#include "gql/ast/nodes/node.h"

namespace gql_demo {

class GQLRuntimeError : public std::runtime_error {
 public:
  GQLRuntimeError(gql::ast::InputPosition, const std::string& msg);

  gql::ast::InputPosition inputPosition() const { return inputPosition_; }
  const std::string& message() const { return msg_; }

 private:
  const gql::ast::InputPosition inputPosition_;
  const std::string msg_;
};

[[noreturn]] void ThrowInvalidValueTypeError(const gql::ast::Node&);
[[noreturn]] void ThrowInvalidReferenceError(const gql::ast::Node&);

}  // namespace gql_demo