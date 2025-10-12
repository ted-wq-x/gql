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

#include "gql/ast/ast.h"
#include "gql/error.h"
#include "gql/gql_export.h"

namespace gql::parser {

class ParserCache;

// Throws ParserError exception on error.
// If cache is nullptr, the parser will use cache in static memory.
GQL_EXPORT ast::GQLProgram ParseProgram(const char* query,
                                        ParserCache* cache = nullptr);

// Antlr4 part of the parser heavily relies on cache to speed up parsing.
// This class may be used to manage memory of the cache instead of relying on
// never-shrinking cache in static memory used by the parser by default.
class GQL_EXPORT ParserCache {
 public:
  ParserCache();
  ~ParserCache();

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;

  friend ast::GQLProgram ParseProgram(const char* query, ParserCache* cache);
};

}  // namespace gql::parser