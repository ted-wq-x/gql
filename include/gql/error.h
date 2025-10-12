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

#ifndef GQL_USE_SHARED
#include <fmt/format.h>
#endif  // GQL_USE_SHARED

#include <stdexcept>
#include <string>

#include "error_codes.h"
#include "gql/ast/nodes/node.h"
#include "gql/error_codes.h"
#include "gql/gql_export.h"

namespace gql {

class GQL_EXPORT ParserError : public std::runtime_error {
 public:
#ifndef GQL_USE_SHARED
  template <typename... Args>
  ParserError(ast::InputPosition pos,
              ErrorCode errorCode,
              const char* defaultFormatString,
              Args&&... args)
      : ParserError(FormattedString(),
                    pos,
                    errorCode,
                    fmt::format(GetFormatString(errorCode, defaultFormatString),
                                std::forward<Args>(args)...)) {}
#endif  // GQL_USE_SHARED

  ast::InputPosition inputPosition() const { return inputPosition_; }
  ErrorCode errorCode() const { return errorCode_; }
  const std::string& message() const { return msg_; }

 private:
  static const char* GetFormatString(ErrorCode errorCode,
                                     const char* defaultFormatString);

  struct FormattedString {};
  ParserError(FormattedString,
              ast::InputPosition,
              ErrorCode errorCode,
              const std::string& msg);

  const ast::InputPosition inputPosition_;
  const ErrorCode errorCode_;
  const std::string msg_;
};

#ifndef GQL_USE_SHARED
class SyntaxRuleError : public ParserError {
 public:
  template <typename... Args>
  SyntaxRuleError(ast::InputPosition pos,
                  ErrorCode errorCode,
                  const char* defaultFormatString,
                  Args&&... args)
      : ParserError(pos,
                    errorCode,
                    defaultFormatString,
                    std::forward<Args>(args)...) {}

  template <typename... Args>
  SyntaxRuleError(const ast::Node& node,
                  ErrorCode errorCode,
                  const char* defaultFormatString,
                  Args&&... args)
      : ParserError(node.inputPosition(),
                    errorCode,
                    defaultFormatString,
                    std::forward<Args>(args)...) {}
};
#endif  // GQL_USE_SHARED

GQL_EXPORT std::string FormatErrorMessageInQuery(const std::string& query,
                                                 gql::ast::InputPosition pos,
                                                 const std::string& message);

}  // namespace gql