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
#include <variant>
#include <vector>

#include "gql/ast/detail/value_ptr.h"

namespace gql_demo {

struct Value;
using ValuePtr = gql::ast::value_ptr<Value>;

class BindingTable;
using BindingTableReference = std::shared_ptr<const BindingTable>;

struct Null {
  bool operator==(const Null&) const { return true; }
};

struct NodeReference {
  int64_t nodeId;

  auto operator<=>(const NodeReference&) const = default;
};

struct EdgeReference {
  int64_t edgeId;

  auto operator<=>(const EdgeReference&) const = default;
};

struct PathReference {
  std::vector<int64_t> nodeIds, edgeIds;

  auto operator<=>(const PathReference&) const = default;
};

using List = std::vector<ValuePtr>;
using Record = std::unordered_map<std::string, Value>;

struct Value {
  Value() = default;
  Value(Value&) = default;
  Value(const Value&) = default;
  Value(Value&&) = default;
  template <typename T>
  Value(T&& val) : v(std::forward<T>(val)) {}

  Value& operator=(const Value&) = default;
  Value& operator=(Value&&) = default;
  bool operator==(const Value&) const = default;
  bool operator!=(const Value&) const = default;

  std::variant<Null,
               double,
               bool,
               int64_t,
               std::string,
               List,
               Record,
               NodeReference,
               EdgeReference,
               PathReference,
               BindingTableReference>
      v;
};

}  // namespace gql_demo
