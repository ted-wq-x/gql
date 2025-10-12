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

#include "gql/ast/algorithm.h"
#include "gql/ast/nodes/expression.h"
#include "gql/ast/print/expression_parens_helpers.h"
#include "gql/ast/print/expression_wrappers.h"
#include "gql/ast/print/helpers.h"
#include "gql/ast/print/output_stream.h"

namespace gql::ast::print {

GQL_AST_ENUM_PRINTER(CompOp,
                     (Equals, "="),
                     (NotEquals, "<>"),
                     (LessThan, "<"),
                     (GreaterThan, ">"),
                     (LessThanOrEquals, "<="),
                     (GreaterThanOrEquals, ">="))

GQL_AST_ENUM_PRINTER_LITERAL(NormalForm, NFC, NFD, NFKC, NFKD)

template <typename Condition>
struct PrintWithParensIf {
  PrintWithParensIf(const ast::ValueExpression* expr) : expr(expr) {}
  PrintWithParensIf(const ValueExpressionPtr& expr) : expr(expr.get()) {}

  const ast::ValueExpression* expr;
};

template <>
struct Printer<BindingTableExpression> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const BindingTableExpression& v) {
    variant_switch(
        v.option, [&os](const ProcedureBodyPtr& v) { os << "{" << *v << "}"; },
        [&os](const ast::ValueExpressionPtr& v) {
          // objectExpressionPrimary
          os << ValueExpressionPrimary(v);
        },
        [&os](const auto& v) { os << v; });
  }
};

template <>
struct Printer<SubCharacterOrByteString> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const SubCharacterOrByteString& v) {
    if (v.direction == SubCharacterOrByteString::Direction::Left) {
      os << "LEFT(";
    } else {
      os << "RIGHT(";
    }
    os << v.expr << "," << v.length << ")";
  }
};

GQL_AST_ENUM_PRINTER_LITERAL(TrimSpecification, LEADING, TRAILING, BOTH)

template <>
struct Printer<TrimSingleCharacterOrByteString> {
  template <typename OutputStream>
  static void Print(OutputStream& os,
                    const TrimSingleCharacterOrByteString& v) {
    os << "TRIM(" << v.specification << v.trimString << "FROM" << v.source
       << ")";
  }
};

template <>
struct Printer<FoldCharacterString> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const FoldCharacterString& v) {
    if (v.case_ == FoldCharacterString::Case::Upper) {
      os << "UPPER(";
    } else {
      os << "LOWER(";
    }
    os << v.expr << ")";
  }
};

template <>
struct Printer<TrimMultiCharacterCharacterString> {
  template <typename OutputStream>
  static void Print(OutputStream& os,
                    const TrimMultiCharacterCharacterString& v) {
    switch (v.type) {
      case TrimMultiCharacterCharacterString::TrimType::BTrim:
        os << "BTRIM(";
        break;
      case TrimMultiCharacterCharacterString::TrimType::LTrim:
        os << "LTRIM(";
        break;
      case TrimMultiCharacterCharacterString::TrimType::RTrim:
        os << "RTRIM(";
        break;
    }
    os << v.source;
    if (v.trimString) {
      os << "," << *v.trimString;
    }
    os << ")";
  }
};

template <>
struct Printer<NormalizeCharacterString> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const NormalizeCharacterString& v) {
    os << "NORMALIZE(" << v.expr << "," << v.form << ")";
  }
};

template <>
struct Printer<PathValueConstructor> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const PathValueConstructor& v) {
    os << "PATH [";
    auto nodeIt = v.nodes.begin();
    for (auto& edge : v.edges) {
      os << ValueExpressionPrimary(*nodeIt) << ","
         << ValueExpressionPrimary(edge) << ",";
      nodeIt++;
    }
    os << ValueExpressionPrimary(*nodeIt) << "]";
  }
};

template <>
struct Printer<PropertyReference> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const PropertyReference& v) {
    os << ValueExpressionPrimary(v.element) << NoBreak() << "." << NoBreak()
       << v.property;
  }
};

GQL_AST_VALUE_PRINTER(SessionUserValue, "SESSION_USER")

template <>
struct Printer<LetVariableDefinition> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const LetVariableDefinition& v) {
    if (v.type) {
      os << "VALUE" << v.var << "TYPED" << v.type << "=" << v.expr;
    } else {
      os << v.var << "=" << v.expr;
    }
  }
};

template <>
struct Printer<LetValueExpression> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const LetValueExpression& v) {
    os << "LET" << Sequence(v.definitions, ",") << "IN" << v.expression
       << "END";
  }
};

template <>
struct Printer<LabelExpression::Negation> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const LabelExpression::Negation& v) {
    os << "!" << NoBreak() << v.expr;
  }
};

template <>
struct Printer<LabelExpression::Logical> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const LabelExpression::Logical& v) {
    os << "("
       << Sequence(v.terms,
                   v.op == LabelExpression::Logical::Op::And ? "&" : "|")
       << ")";
  }
};

GQL_AST_VALUE_PRINTER(LabelExpression::Wildcard, "%")

template <>
struct Printer<WhenOperand> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const WhenOperand& w) {
    variant_switch(
        w.option,
        [&os](const ValueExpressionPtr& v) { os << ValueExpressionPrimary(v); },
        [&os](const WhenOperand::Comparison& v) { os << v.op << v.value; },
        [&](const WhenOperand::IsNull&) {
          os << "IS";
          if (w.isNot) {
            os << "NOT";
          }
          os << "NULL";
        },
        [&](const WhenOperand::IsTyped& v) {
          os << "IS";
          if (w.isNot) {
            os << "NOT";
          }
          os << "TYPED" << v.type;
        },
        [&](const WhenOperand::IsNormalized& v) {
          os << "IS";
          if (w.isNot) {
            os << "NOT";
          }
          os << v.form << "NORMALIZED";
        },
        [&](const WhenOperand::IsDirected&) {
          os << "IS";
          if (w.isNot) {
            os << "NOT";
          }
          os << "DIRECTED";
        },
        [&](const WhenOperand::IsLabeled& v) {
          if (w.isNot) {
            os << "IS NOT LABELED";
          } else {
            os << ":" << NoBreak();
          }
          os << v.label;
        },
        [&](const WhenOperand::IsSourceOrDestinationOf& v) {
          os << "IS";
          if (w.isNot) {
            os << "NOT";
          }
          switch (v.direction) {
            case WhenOperand::IsSourceOrDestinationOf::Direction::Source:
              os << "SOURCE";
              break;
            case WhenOperand::IsSourceOrDestinationOf::Direction::Destination:
              os << "DESTINATION";
              break;
          }
          os << "OF" << v.edge;
        });
  }
};

template <>
struct Printer<NullIfCaseAbbreviation> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const NullIfCaseAbbreviation& v) {
    os << "NULLIF(" << v.first << "," << v.second << ")";
  }
};

template <>
struct Printer<CoalesceCaseAbbreviation> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const CoalesceCaseAbbreviation& v) {
    os << "COALESCE(" << Sequence(v.expressions, ",") << ")";
  }
};

template <>
struct Printer<SimpleWhenClause> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const SimpleWhenClause& v) {
    os << "WHEN" << Sequence(v.operands, ",") << "THEN" << v.result;
  }
};

template <>
struct Printer<SimpleCase> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const SimpleCase& v) {
    os << "CASE" << v.operand << v.when;
    if (v.else_) {
      os << "ELSE" << *v.else_;
    }
    os << "END";
  }
};

template <>
struct Printer<SearchedWhenClause> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const SearchedWhenClause& v) {
    os << "WHEN" << v.condition << "THEN" << v.result;
  }
};

template <>
struct Printer<SearchedCase> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const SearchedCase& v) {
    os << "CASE" << v.when;
    if (v.else_) {
      os << "ELSE" << *v.else_;
    }
    os << "END";
  }
};

template <>
struct Printer<CastSpecification> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const CastSpecification& v) {
    os << "CAST(" << v.operand << "AS" << v.target << ")";
  }
};

GQL_AST_VALUE_PRINTER(CountAsteriskValue, "COUNT(*)")

GQL_AST_ENUM_PRINTER_LITERAL(GeneralSetFunctionType,
                             AVG,
                             COUNT,
                             MAX,
                             MIN,
                             SUM,
                             COLLECT_LIST,
                             STDDEV_SAMP,
                             STDDEV_POP)

GQL_AST_ENUM_PRINTER_LITERAL(SetQuantifier, DISTINCT, ALL)

template <>
struct Printer<GeneralSetFunction> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const GeneralSetFunction& v) {
    os << v.type << NoBreak() << "(" << v.quantifier << v.value << ")";
  }
};

GQL_AST_ENUM_PRINTER_LITERAL(BinarySetFunctionType,
                             PERCENTILE_CONT,
                             PERCENTILE_DISC)

template <>
struct Printer<BinarySetFunction> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const BinarySetFunction& v) {
    os << v.type << NoBreak() << "(" << v.quantifier
       << PrintWithParensIf<NumericValueExpressionRequiresParens>(
              v.dependentValue)
       << ","
       << PrintWithParensIf<NumericValueExpressionRequiresParens>(v.independent)
       << ")";
  }
};

template <>
struct Printer<ElementIdFunction> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const ElementIdFunction& v) {
    os << "ELEMENT_ID(" << v.variable << ")";
  }
};

template <>
struct Printer<DateTimeFunction> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const DateTimeFunction& v) {
    switch (v.function) {
      case DateTimeFunction::Function::LOCAL_TIME:
        os << "LOCAL_TIME";
        break;
      case DateTimeFunction::Function::ZONED_TIME:
        os << "ZONED_TIME";
        break;
      case DateTimeFunction::Function::ZONED_DATETIME:
        os << "ZONED_DATETIME";
        break;
      case DateTimeFunction::Function::LOCAL_DATETIME:
        os << "LOCAL_DATETIME";
        break;
      case DateTimeFunction::Function::DATE:
        os << "DATE";
        break;
      case DateTimeFunction::Function::DURATION:
        os << "DURATION";
        break;
    }
    os << NoBreak() << "(" << v.parameters << ")";
  }
};

template <>
struct Printer<DatetimeSubtraction> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const DatetimeSubtraction& v) {
    os << "DURATION_BETWEEN(" << v.param1 << "," << v.param2 << ")"
       << v.qualifier;
  }
};

template <>
struct Printer<NormalizedPredicate> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const NormalizedPredicate& v) {
    os << v.expr << "IS";
    if (v.isNot) {
      os << "NOT";
    }
    os << v.form << "NORMALIZED";
  }
};

GQL_AST_VALUE_PRINTER(CurrentGraph, "CURRENT_GRAPH")

template <>
struct Printer<ExistsPredicate> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const ExistsPredicate& v) {
    os << "EXISTS {" << v.option << "}";
  }
};

template <>
struct Printer<NullPredicate> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const NullPredicate& v) {
    os << ValueExpressionPrimary(v.expr) << "IS";
    if (v.isNot)
      os << "NOT";
    os << "NULL";
  }
};

template <>
struct Printer<ValueTypePredicate> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const ValueTypePredicate& v) {
    os << ValueExpressionPrimary(v.expr) << "IS";
    if (v.isNot)
      os << "NOT";
    os << "TYPED" << v.type;
  }
};

template <>
struct Printer<DirectedPredicate> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const DirectedPredicate& v) {
    os << v.element << "IS";
    if (v.isNot)
      os << "NOT";
    os << "DIRECTED";
  }
};

template <>
struct Printer<LabeledPredicate> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const LabeledPredicate& v) {
    os << v.element;
    if (v.isNot)
      os << "IS NOT LABELED";
    else
      os << ":" << NoBreak();
    os << v.label;
  }
};

template <>
struct Printer<SourceDestinationPredicate> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const SourceDestinationPredicate& v) {
    os << v.node << "IS";
    if (v.isNot)
      os << "NOT";
    if (v.kind == SourceDestinationPredicate::Kind::NodeIsSourceOfEdge)
      os << "SOURCE OF";
    else
      os << "DESTINATION OF";

    os << v.edge;
  }
};

template <>
struct Printer<AllElementsPredicate> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const AllElementsPredicate& v) {
    switch (v.kind) {
      case AllElementsPredicate::Kind::AllDifferent:
        os << "ALL_DIFFERENT(";
        break;
      case AllElementsPredicate::Kind::Same:
        os << "SAME(";
        break;
    }
    os << Sequence(v.elements, ",") << ")";
  }
};

template <>
struct Printer<PropertyExistsPredicate> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const PropertyExistsPredicate& v) {
    os << "PROPERTY_EXISTS(" << v.element << "," << v.property << ")";
  }
};

template <>
struct Printer<ast::ValueExpression::Unary> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const ast::ValueExpression::Unary& v) {
    switch (v.op) {
      case ast::ValueExpression::Unary::Op::Positive:
        os << "+" << NoBreak()
           << PrintWithParensIf<NumericPrimaryRequiresParens>(v.expr);
        break;
      case ast::ValueExpression::Unary::Op::Negative:
        os << "-" << NoBreak()
           << PrintWithParensIf<NumericPrimaryRequiresParens>(v.expr);
        break;
      case ast::ValueExpression::Unary::Op::BoolNot:
        os << "NOT" << PrintWithParensIf<BooleanTestRequiresParens>(v.expr);
        break;
      case ast::ValueExpression::Unary::Op::Sin:
        os << "SIN("
           << PrintWithParensIf<NumericValueExpressionRequiresParens>(v.expr)
           << ")";
        break;
      case ast::ValueExpression::Unary::Op::Cos:
        os << "COS("
           << PrintWithParensIf<NumericValueExpressionRequiresParens>(v.expr)
           << ")";
        break;
      case ast::ValueExpression::Unary::Op::Tan:
        os << "TAN("
           << PrintWithParensIf<NumericValueExpressionRequiresParens>(v.expr)
           << ")";
        break;
      case ast::ValueExpression::Unary::Op::Cot:
        os << "COT("
           << PrintWithParensIf<NumericValueExpressionRequiresParens>(v.expr)
           << ")";
        break;
      case ast::ValueExpression::Unary::Op::Sinh:
        os << "SINH("
           << PrintWithParensIf<NumericValueExpressionRequiresParens>(v.expr)
           << ")";
        break;
      case ast::ValueExpression::Unary::Op::Cosh:
        os << "COSH("
           << PrintWithParensIf<NumericValueExpressionRequiresParens>(v.expr)
           << ")";
        break;
      case ast::ValueExpression::Unary::Op::Tanh:
        os << "TANH("
           << PrintWithParensIf<NumericValueExpressionRequiresParens>(v.expr)
           << ")";
        break;
      case ast::ValueExpression::Unary::Op::Asin:
        os << "ASIN("
           << PrintWithParensIf<NumericValueExpressionRequiresParens>(v.expr)
           << ")";
        break;
      case ast::ValueExpression::Unary::Op::Acos:
        os << "ACOS("
           << PrintWithParensIf<NumericValueExpressionRequiresParens>(v.expr)
           << ")";
        break;
      case ast::ValueExpression::Unary::Op::Atan:
        os << "ATAN("
           << PrintWithParensIf<NumericValueExpressionRequiresParens>(v.expr)
           << ")";
        break;
      case ast::ValueExpression::Unary::Op::Degrees:
        os << "DEGREES("
           << PrintWithParensIf<NumericValueExpressionRequiresParens>(v.expr)
           << ")";
        break;
      case ast::ValueExpression::Unary::Op::Radians:
        os << "RADIANS("
           << PrintWithParensIf<NumericValueExpressionRequiresParens>(v.expr)
           << ")";
        break;
      case ast::ValueExpression::Unary::Op::Floor:
        os << "FLOOR("
           << PrintWithParensIf<NumericValueExpressionRequiresParens>(v.expr)
           << ")";
        break;
      case ast::ValueExpression::Unary::Op::Ceiling:
        os << "CEILING("
           << PrintWithParensIf<NumericValueExpressionRequiresParens>(v.expr)
           << ")";
        break;
      case ast::ValueExpression::Unary::Op::SquareRoot:
        os << "SQRT("
           << PrintWithParensIf<NumericValueExpressionRequiresParens>(v.expr)
           << ")";
        break;
      case ast::ValueExpression::Unary::Op::Exponential:
        os << "EXP("
           << PrintWithParensIf<NumericValueExpressionRequiresParens>(v.expr)
           << ")";
        break;
      case ast::ValueExpression::Unary::Op::NaturalLogarithm:
        os << "LN("
           << PrintWithParensIf<NumericValueExpressionRequiresParens>(v.expr)
           << ")";
        break;
      case ast::ValueExpression::Unary::Op::CommonLogarithm:
        os << "LOG10("
           << PrintWithParensIf<NumericValueExpressionRequiresParens>(v.expr)
           << ")";
        break;
      case ast::ValueExpression::Unary::Op::AbsoluteValue:
        os << "ABS(" << v.expr << ")";
        break;
      case ast::ValueExpression::Unary::Op::CharLength:
        os << "CHAR_LENGTH(" << v.expr << ")";
        break;
      case ast::ValueExpression::Unary::Op::ByteLength:
        os << "BYTE_LENGTH(" << v.expr << ")";
        break;
      case ast::ValueExpression::Unary::Op::PathLength:
        os << "PATH_LENGTH(" << v.expr << ")";
        break;
      case ast::ValueExpression::Unary::Op::Cardinality:
        os << "CARDINALITY(" << v.expr << ")";
        break;
      case ast::ValueExpression::Unary::Op::Size:
        os << "SIZE(" << v.expr << ")";
        break;
      case ast::ValueExpression::Unary::Op::Elements:
        os << "ELEMENTS(" << v.expr << ")";
        break;
    }
  }
};

template <>
struct Printer<ast::ValueExpression::Binary> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const ast::ValueExpression::Binary& v) {
    switch (v.op) {
      case ast::ValueExpression::Binary::Op::Multiply:
        os << PrintWithParensIf<NumericTermRequiresParens>(v.left) << "*"
           << PrintWithParensIf<NumericFactorRequiresParens>(v.right);
        break;
      case ast::ValueExpression::Binary::Op::Divide:
        os << PrintWithParensIf<NumericTermRequiresParens>(v.left) << "/"
           << PrintWithParensIf<NumericFactorRequiresParens>(v.right);
        break;
      case ast::ValueExpression::Binary::Op::Add:
        os << PrintWithParensIf<NumericValueExpressionRequiresParens>(v.left)
           << "+" << PrintWithParensIf<NumericTermRequiresParens>(v.right);
        break;
      case ast::ValueExpression::Binary::Op::Subtract:
        os << PrintWithParensIf<NumericValueExpressionRequiresParens>(v.left)
           << "-" << PrintWithParensIf<NumericTermRequiresParens>(v.right);
        break;
      case ast::ValueExpression::Binary::Op::Concatenate:
        os << PrintWithParensIf<ConcatenateRequiresParens>(v.left) << "||"
           << ValueExpressionPrimary(v.right);
        break;
      case ast::ValueExpression::Binary::Op::BoolAnd:
        os << PrintWithParensIf<BooleanTermRequiresParens>(v.left) << "AND"
           << PrintWithParensIf<BooleanFactorRequiresParens>(v.right);
        break;
      case ast::ValueExpression::Binary::Op::BoolOr:
        os << PrintWithParensIf<BooleanValueExpressionRequiresParens>(v.left)
           << "OR" << PrintWithParensIf<BooleanTermRequiresParens>(v.right);
        break;
      case ast::ValueExpression::Binary::Op::BoolXor:
        os << PrintWithParensIf<BooleanValueExpressionRequiresParens>(v.left)
           << "XOR" << PrintWithParensIf<BooleanTermRequiresParens>(v.right);
        break;
      case ast::ValueExpression::Binary::Op::Power:
        os << "POWER("
           << PrintWithParensIf<NumericValueExpressionRequiresParens>(v.left)
           << ","
           << PrintWithParensIf<NumericValueExpressionRequiresParens>(v.right)
           << ")";
        break;
      case ast::ValueExpression::Binary::Op::GeneralLogarithm:
        os << "LOG("
           << PrintWithParensIf<NumericValueExpressionRequiresParens>(v.left)
           << ","
           << PrintWithParensIf<NumericValueExpressionRequiresParens>(v.right)
           << ")";
        break;
      case ast::ValueExpression::Binary::Op::Modulus:
        os << "MOD("
           << PrintWithParensIf<NumericValueExpressionRequiresParens>(v.left)
           << ","
           << PrintWithParensIf<NumericValueExpressionRequiresParens>(v.right)
           << ")";
        break;
      case ast::ValueExpression::Binary::Op::TrimList:
        os << "TRIM(" << v.left << "," << v.right << ")";
        break;
    }
  }
};

template <>
struct Printer<ast::ValueExpression::Comparison> {
  template <typename OutputStream>
  static void Print(OutputStream& os,
                    const ast::ValueExpression::Comparison& v) {
    os << v.left << v.op << v.right;
  }
};

template <>
struct Printer<ast::ValueExpression::Is> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const ast::ValueExpression::Is& v) {
    os << PrintWithParensIf<BooleanPrimaryRequiresParens>(v.expr) << "IS";
    if (v.isNot)
      os << "NOT";
    os << v.value;
  }
};

template <>
struct Printer<ast::ValueExpression> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const ValueExpression& v) {
    variant_switch(
        v.option,
        [&os](const BindingTableExpression& v) { os << "TABLE" << v; },
        [&os](const GraphExpression& v) { os << "GRAPH" << v; },
        [&os](const ProcedureBodyPtr& v) { os << "VALUE {" << *v << "}"; },
        [&os](const auto& v) { os << v; });
  }
};

template <typename Condition>
struct Printer<PrintWithParensIf<Condition>> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const PrintWithParensIf<Condition>& v) {
    if (std::visit(Condition{}, v.expr->option)) {
      os << "(" << *v.expr << ")";
    } else {
      os << *v.expr;
    }
  }
};

template <>
struct Printer<ValueExpressionPrimary> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const ValueExpressionPrimary& v) {
    os << PrintWithParensIf<ValueExpressionRequiresParens>(v.expr);
  }
};

template <>
struct Printer<GraphExpression> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const GraphExpression& v) {
    variant_switch(
        v.option,
        [&os](const ast::ValueExpressionPtr& v) {
          // objectExpressionPrimary
          os << "VARIABLE" << ValueExpressionPrimary(v);
        },
        [&os](const auto& v) { os << v; });
  }
};

}  // namespace gql::ast::print
