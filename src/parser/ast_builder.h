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

#include "generated/GQLParser.h"
#include "gql/ast/ast.h"
#include "gql/parser/parser.h"

namespace gql::parser {

void BuildAST(GQLParser::GqlProgramContext*,
              ast::GQLProgram&,
              const ParserConfig&);

void BuildASTForTesting(GQLParser::UnsignedNumericLiteralContext* ctx,
                        ast::UnsignedNumericLiteral& value);
void BuildASTForTesting(GQLParser::CharacterStringLiteralContext* ctx,
                        std::string& value);
void BuildASTForTesting(GQLParser::ValueTypeContext* ctx,
                        ast::ValueType& value);
void BuildASTForTesting(GQLParser::ValueExpressionContext* ctx,
                        ast::ValueExpression& value);

}  // namespace gql::parser
