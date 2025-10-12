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
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "gql/ast/detail/reflection.h"
#include "gql/ast/detail/value_ptr.h"


namespace gql::ast {

template <typename T, class Enable = void>
struct EnumerateChildren {};

template <typename T>
struct EnumerateChildren<const T> : EnumerateChildren<T> {};

template <typename T>
struct EnumerateChildren<T, typename SupportsEnumerateFields<T>::type> {
  template <typename ObjectType, typename VisitorType>
  static bool ForEachChild(ObjectType& obj, VisitorType&& visitor) {
    return EnumerateFields<T>::ForEachField(
        obj, [&visitor](auto& field) { return visitor(field); });
  }
};

template <>
struct EnumerateChildren<std::string> {
  template <typename VisitorType>
  static bool ForEachChild(const std::string&, VisitorType&&) {
    return true;
  }
};

template <typename T>
struct EnumerateChildren<
    T,
    std::enable_if_t<std::is_arithmetic_v<T> && !std::is_const_v<T>>> {
  template <typename VisitorType>
  static bool ForEachChild(const T&, VisitorType&&) {
    return true;
  }
};

template <typename T>
struct EnumerateChildren<
    T,
    std::enable_if_t<std::is_enum_v<T> && !std::is_const_v<T>>> {
  template <typename VisitorType>
  static bool ForEachChild(const T&, VisitorType&&) {
    return true;
  }
};

template <typename T>
struct EnumerateChildren<std::optional<T>> {
  template <typename ObjectType, typename VisitorType>
  static bool ForEachChild(ObjectType& o, VisitorType&& visitor) {
    if (!o)
      return true;
    return visitor(*o);
  }
};

template <typename T>
struct EnumerateChildren<value_ptr<T>> {
  template <typename VisitorType>
  static bool ForEachChild(value_ptr<T>& o, VisitorType&& visitor) {
    return visitor(*o);
  }

  template <typename VisitorType>
  static bool ForEachChild(const value_ptr<T>& o, VisitorType&& visitor) {
    return visitor(const_cast<const T&>(*o));
  }
};

template <typename T>
struct EnumerateChildren<std::vector<T>> {
  template <typename ObjectType, typename VisitorType>
  static bool ForEachChild(ObjectType& o, VisitorType&& visitor) {
    for (auto& item : o) {
      if (!visitor(item))
        return false;
    }
    return true;
  }
};

template <typename... Ts>
struct EnumerateChildren<std::variant<Ts...>> {
  template <typename ObjectType, typename VisitorType>
  static bool ForEachChild(ObjectType& v, VisitorType&& visitor) {
    return std::visit([&visitor](auto& arg) { return visitor(arg); }, v);
  }
};

}  // namespace gql::ast
