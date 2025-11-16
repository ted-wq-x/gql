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

#include "gql/ast/nodes/expression.h"

namespace gql::ast::print {

// Check for <non-parenthesized value expression primary>
struct ValueExpressionRequiresParens {
  bool operator()(const BindingVariableReference&) { return false; }
  bool operator()(const AggregateFunction&) { return false; }
  bool operator()(const PathValueConstructor&) { return false; }
  bool operator()(const PropertyReference&) { return false; }
  bool operator()(const ProcedureBodyPtr&) { return false; }
  bool operator()(const CaseExpression&) { return false; }
  bool operator()(const CastSpecification&) { return false; }
  bool operator()(const ElementIdFunction&) { return false; }
  bool operator()(const LetValueExpression&) { return false; }

  // <unsigned value specification>
  bool operator()(const UnsignedLiteral&) { return false; }
  bool operator()(const GeneralParameterReference&) { return false; }
  bool operator()(const SessionUserValue&) { return false; }

  template <class T>
  bool operator()(const T&) {
    return true;
  }
};

using BooleanPredicandRequiresParens = ValueExpressionRequiresParens;

struct BooleanPrimaryRequiresParens {
  bool operator()(const ast::ValueExpression::Comparison&) { return false; }
  bool operator()(const ast::Predicate&) { return false; }
  bool operator()(const NormalizedPredicate&) { return false; }

  template <class T>
  bool operator()(const T& v) {
    return BooleanPredicandRequiresParens()(v);
  }
};

struct BooleanTestRequiresParens {
  bool operator()(const ast::ValueExpression::BooleanTest&) { return false; }

  template <class T>
  bool operator()(const T& v) {
    return BooleanPrimaryRequiresParens()(v);
  }
};

struct BooleanFactorRequiresParens {
  bool operator()(const ast::ValueExpression::Unary& v) {
    if (v.op == ast::ValueExpression::Unary::Op::BoolNot)
      return false;
    return BooleanTestRequiresParens()(v);
  }

  template <class T>
  bool operator()(const T& v) {
    return BooleanTestRequiresParens()(v);
  }
};

struct BooleanTermRequiresParens {
  bool operator()(const ast::ValueExpression::Binary& v) {
    if (v.op == ast::ValueExpression::Binary::Op::BoolAnd)
      return false;
    return BooleanFactorRequiresParens()(v);
  }

  template <class T>
  bool operator()(const T& v) {
    return BooleanFactorRequiresParens()(v);
  }
};

struct BooleanValueExpressionRequiresParens {
  bool operator()(const ast::ValueExpression::Binary& v) {
    if (v.op == ast::ValueExpression::Binary::Op::BoolOr ||
        v.op == ast::ValueExpression::Binary::Op::BoolXor)
      return false;
    return BooleanTermRequiresParens()(v);
  }

  template <class T>
  bool operator()(const T& v) {
    return BooleanTermRequiresParens()(v);
  }
};

struct NumericPrimaryRequiresParens {
  // <numeric value function>
  bool operator()(const ast::ValueExpression::Unary& v) {
    switch (v.op) {
      case ast::ValueExpression::Unary::Op::CharLength:
      case ast::ValueExpression::Unary::Op::ByteLength:
      case ast::ValueExpression::Unary::Op::PathLength:
      case ast::ValueExpression::Unary::Op::Cardinality:
      case ast::ValueExpression::Unary::Op::Size:
      case ast::ValueExpression::Unary::Op::AbsoluteValue:
      case ast::ValueExpression::Unary::Op::Sin:
      case ast::ValueExpression::Unary::Op::Cos:
      case ast::ValueExpression::Unary::Op::Tan:
      case ast::ValueExpression::Unary::Op::Cot:
      case ast::ValueExpression::Unary::Op::Sinh:
      case ast::ValueExpression::Unary::Op::Cosh:
      case ast::ValueExpression::Unary::Op::Tanh:
      case ast::ValueExpression::Unary::Op::Asin:
      case ast::ValueExpression::Unary::Op::Acos:
      case ast::ValueExpression::Unary::Op::Atan:
      case ast::ValueExpression::Unary::Op::Degrees:
      case ast::ValueExpression::Unary::Op::Radians:
      case ast::ValueExpression::Unary::Op::NaturalLogarithm:
      case ast::ValueExpression::Unary::Op::CommonLogarithm:
      case ast::ValueExpression::Unary::Op::Exponential:
      case ast::ValueExpression::Unary::Op::SquareRoot:
      case ast::ValueExpression::Unary::Op::Floor:
      case ast::ValueExpression::Unary::Op::Ceiling:
        return false;
      default:
        return ValueExpressionRequiresParens()(v);
    }
  }

  bool operator()(const ast::ValueExpression::Binary& v) {
    switch (v.op) {
      case ast::ValueExpression::Binary::Op::Modulus:
      case ast::ValueExpression::Binary::Op::GeneralLogarithm:
      case ast::ValueExpression::Binary::Op::Power:
        return false;
      default:
        return ValueExpressionRequiresParens()(v);
    }
  }

  template <class T>
  bool operator()(const T& v) {
    return ValueExpressionRequiresParens()(v);
  }
};

struct NumericFactorRequiresParens {
  bool operator()(const ast::ValueExpression::Unary& v) {
    if (v.op == ast::ValueExpression::Unary::Op::Positive ||
        v.op == ast::ValueExpression::Unary::Op::Negative)
      return false;
    return NumericPrimaryRequiresParens()(v);
  }

  template <class T>
  bool operator()(const T& v) {
    return NumericPrimaryRequiresParens()(v);
  }
};

struct NumericTermRequiresParens {
  bool operator()(const ast::ValueExpression::Binary& v) {
    if (v.op == ast::ValueExpression::Binary::Op::Multiply ||
        v.op == ast::ValueExpression::Binary::Op::Divide)
      return false;
    return NumericFactorRequiresParens()(v);
  }

  template <class T>
  bool operator()(const T& v) {
    return NumericFactorRequiresParens()(v);
  }
};

struct NumericValueExpressionRequiresParens {
  bool operator()(const ast::ValueExpression::Binary& v) {
    if (v.op == ast::ValueExpression::Binary::Op::Add ||
        v.op == ast::ValueExpression::Binary::Op::Subtract)
      return false;
    return NumericTermRequiresParens()(v);
  }

  template <class T>
  bool operator()(const T& v) {
    return NumericTermRequiresParens()(v);
  }
};

struct ConcatenateRequiresParens {
  bool operator()(const ast::ValueExpression::Binary& v) {
    if (v.op == ast::ValueExpression::Binary::Op::Concatenate)
      return false;
    return ValueExpressionRequiresParens()(v);
  }

  template <class T>
  bool operator()(const T& v) {
    return ValueExpressionRequiresParens()(v);
  }
};

}  // namespace gql::ast::print
