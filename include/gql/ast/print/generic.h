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

#include <optional>
#include <type_traits>
#include <variant>
#include <vector>

#include "gql/ast/detail/reflection.h"
#include "gql/ast/detail/value_ptr.h"
#include "gql/ast/print/output_stream.h"


namespace gql::ast::print {

template <>
struct Printer<std::string> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const std::string& s) {
    os << Quoted(s);
  }
};

template <typename T>
struct Printer<std::optional<T>> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const std::optional<T>& o) {
    if (o)
      os << *o;
  }
};

template <typename T>
struct Printer<value_ptr<T>> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const value_ptr<T>& o) {
    os << *o;
  }
};

template <typename T>
struct Printer<std::vector<T>> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const std::vector<T>& o) {
    for (auto& item : o) {
      os << item;
    }
  }
};

template <typename... Ts>
struct Printer<std::variant<Ts...>> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const std::variant<Ts...>& v) {
    std::visit([&os](auto& arg) { os << arg; }, v);
  }
};

template <typename T>
constexpr bool HasPrinterSpecialization =
    std::is_base_of_v<BindingVariableBase, T> ||
    std::is_base_of_v<IdentifierBase, T>;

template <typename T>
struct Printer<T,
               std::enable_if_t<SupportsEnumerateFields<T>::value &&
                                !HasPrinterSpecialization<T>>> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const T& obj) {
    EnumerateFields<T>::ForEachField(obj, [&os](auto& field) {
      os << field;
      return true;
    });
  }
};

}  // namespace gql::ast::print
