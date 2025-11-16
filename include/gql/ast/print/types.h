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

#include "gql/ast/nodes/types.h"
#include "gql/ast/print/helpers.h"
#include "gql/ast/print/output_stream.h"

namespace gql::ast::print {

GQL_AST_ENUM_PRINTER(SimplePredefinedType,
                     (Boolean, "BOOL"),
                     (Datetime, "ZONED DATETIME"),
                     (Localdatetime, "LOCAL DATETIME"),
                     (Date, "DATE"),
                     (Time, "ZONED TIME"),
                     (Localtime, "LOCAL TIME"),
                     (DurationYearToMonth, "DURATION(YEAR TO MONTH)"),
                     (DurationDayToSecond, "DURATION(DAY TO SECOND)"),
                     (Null, "NULL"),
                     (Empty, "NOTHING"),
                     (Any, "ANY VALUE"),
                     (AnyProperty, "ANY PROPERTY VALUE"),
                     (Path, "PATH"))

template <>
struct Printer<StringType> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const StringType& v) {
    if (v.maxLength && *v.maxLength == v.minLength) {
      // Fixed length
      os << (v.kind == StringType::Kind::CHAR ? "CHAR" : "BINARY") << "("
         << v.minLength << ")";
    } else if (!v.maxLength) {
      os << (v.kind == StringType::Kind::CHAR ? "CHAR" : "BINARY");
    } else {
      os << (v.kind == StringType::Kind::CHAR ? "STRING" : "BYTES") << "(";
      if (v.minLength > 0) {
        os << v.minLength << ",";
      }
      os << v.maxLength << ")";
    }
  }
};

GQL_AST_ENUM_PRINTER(SimpleNumericType,
                     (Float16, "FLOAT16"),
                     (Float32, "FLOAT32"),
                     (Float64, "FLOAT64"),
                     (Float128, "FLOAT128"),
                     (Float256, "FLOAT256"),
                     (Real, "REAL"),
                     (Double, "DOUBLE"))

template <>
struct Printer<BinaryExactNumericType> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const BinaryExactNumericType& v) {
    switch (v.isSigned ? v.precision + 1 : v.precision) {
      case 8:
        os << (v.isSigned ? "INT8" : "UINT8");
        return;
      case 16:
        os << (v.isSigned ? "INT16" : "UINT16");
        return;
      case 32:
        os << (v.isSigned ? "INT32" : "UINT32");
        return;
      case 64:
        os << (v.isSigned ? "INT64" : "UINT64");
        return;
      case 128:
        os << (v.isSigned ? "INT128" : "UINT128");
        return;
      case 256:
        os << (v.isSigned ? "INT256" : "UINT256");
        return;
      default:
        os << (v.isSigned ? "INT" : "UINT") << "(" << v.precision << ")";
        break;
    }
  }
};

GQL_AST_ENUM_PRINTER(ScaleNumericType::Type,
                     (Decimal, "DECIMAL"),
                     (Float, "FLOAT"))

template <>
struct Printer<ScaleNumericType> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const ScaleNumericType& v) {
    os << v.type;
    if (v.precision) {
      os << "(" << *v.precision;
      if (v.scale)
        os << "," << *v.scale;
      os << ")";
    }
  }
};

GQL_AST_ENUM_PRINTER(TemporalDurationQualifier,
                     (YearToMonth, "YEAR TO MONTH"),
                     (DayToSecond, "DAY TO SECOND"))

template <>
struct Printer<FieldType> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const FieldType& v) {
    os << v.name << "TYPED" << v.type;
  }
};

template <>
struct Printer<FieldTypeList> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const FieldTypeList& v) {
    os << Sequence(v, ",");
  }
};

template <>
struct Printer<FieldTypesSpecification> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const FieldTypesSpecification& v) {
    os << "{" << v.fields << "}";
  }
};

template <>
struct Printer<BindingTableReferenceValueType> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const BindingTableReferenceValueType& v) {
    os << "TABLE" << v.type;
  }
};

template <>
struct Printer<GraphReferenceValueType> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const GraphReferenceValueType& v) {
    if (v.type)
      os << "GRAPH {" << *v.type << "}";
    else
      os << "ANY GRAPH";
  }
};

template <>
struct Printer<NodeReferenceValueType> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const NodeReferenceValueType& v) {
    if (v.type)
      os << v.type;
    else
      os << "ANY NODE";
  }
};

template <>
struct Printer<EdgeReferenceValueType> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const EdgeReferenceValueType& v) {
    if (v.type)
      os << v.type;
    else
      os << "ANY EDGE";
  }
};

template <>
struct Printer<RecordType> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const RecordType& v) {
    if (v.fields)
      os << "RECORD" << *v.fields;
    else
      os << "ANY RECORD";
  }
};

template <>
struct Printer<ValueType> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const ValueType& v) {
    os << v.typeOption;
    if (v.notNull)
      os << "NOT NULL";
  }
};

template <>
struct Printer<ValueType::List> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const ValueType::List& v) {
    if (v.isGroup)
      os << "GROUP";
    os << "LIST" << "<" << NoBreak() << v.valueType << NoBreak() << ">";
    if (v.maxLength)
      os << "[" << *v.maxLength << "]";
  }
};

template <>
struct Printer<ValueType::Union> {
  template <typename OutputStream>
  static void Print(OutputStream& os, const ValueType::Union& v) {
    os << "ANY <" << NoBreak() << Sequence(v.types, " | ") << NoBreak() << ">";
  }
};

}  // namespace gql::ast::print
