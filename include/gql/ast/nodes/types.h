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

#include "gql/ast/nodes/graph_types.h"

namespace gql::ast {

enum class SimplePredefinedType {
  Boolean,
  Datetime,
  Localdatetime,
  Date,
  Time,
  Localtime,
  DurationYearToMonth,
  DurationDayToSecond,
  Null,
  Empty,
  Any,
  AnyProperty,
  Path,
};

// booleanType
//     : (BOOL | BOOLEAN) notNull?
//     ;

// characterStringType
//     : STRING (LEFT_PAREN (minLength COMMA)? maxLength RIGHT_PAREN)? notNull?
//     | CHAR (LEFT_PAREN fixedLength RIGHT_PAREN)? notNull?
//     | VARCHAR (LEFT_PAREN maxLength RIGHT_PAREN)? notNull?
//     ;
struct StringType {
  enum class Kind { CHAR, BYTES };

  Kind kind = Kind::CHAR;
  UnsignedInteger minLength = 0;
  std::optional<UnsignedInteger> maxLength;
};
GQL_AST_STRUCT(StringType, kind, minLength, maxLength)

// byteStringType
//     : BYTES (LEFT_PAREN (minLength COMMA)? maxLength RIGHT_PAREN)? notNull?
//     | BINARY (LEFT_PAREN fixedLength RIGHT_PAREN)? notNull?
//     | VARBINARY (LEFT_PAREN maxLength RIGHT_PAREN)? notNull?
//     ;

// minLength
//     : unsignedInteger
//     ;

// maxLength
//     : unsignedInteger
//     ;

// fixedLength
//     : unsignedInteger
//     ;

enum class SimpleNumericType {
  Float16,
  Float32,
  Float64,
  Float128,
  Float256,
  Real,
  Double
};

struct BinaryExactNumericType {
  bool isSigned = false;
  uint64_t precision = 0;
};
GQL_AST_STRUCT(BinaryExactNumericType, isSigned, precision)

struct ScaleNumericType {
  enum class Type { Decimal, Float };

  Type type;
  std::optional<uint64_t> precision;
  std::optional<uint64_t> scale;
};
GQL_AST_STRUCT(ScaleNumericType, type, precision, scale)

// numericType
//     : exactNumericType
//     | approximateNumericType
//     ;

// exactNumericType
//     : binaryExactNumericType
//     | decimalExactNumericType
//     ;

// binaryExactNumericType
//     : signedBinaryExactNumericType
//     | unsignedBinaryExactNumericType
//     ;

// signedBinaryExactNumericType
//     : INT8 notNull?
//     | INT16 notNull?
//     | INT32 notNull?
//     | INT64 notNull?
//     | INT128 notNull?
//     | INT256 notNull?
//     | SMALLINT notNull?
//     | INT (LEFT_PAREN precision RIGHT_PAREN)? notNull?
//     | BIGINT notNull?
//     | SIGNED? verboseBinaryExactNumericType
//     ;

// unsignedBinaryExactNumericType
//     : UINT8 notNull?
//     | UINT16 notNull?
//     | UINT32 notNull?
//     | UINT64 notNull?
//     | UINT128 notNull?
//     | UINT256 notNull?
//     | USMALLINT notNull?
//     | UINT (LEFT_PAREN precision RIGHT_PAREN)? notNull?
//     | UBIGINT notNull?
//     | UNSIGNED verboseBinaryExactNumericType
//     ;

// verboseBinaryExactNumericType
//     : INTEGER8 notNull?
//     | INTEGER16 notNull?
//     | INTEGER32 notNull?
//     | INTEGER64 notNull?
//     | INTEGER128 notNull?
//     | INTEGER256 notNull?
//     | SMALL INTEGER notNull?
//     | INTEGER (LEFT_PAREN precision RIGHT_PAREN)? notNull?
//     | BIG INTEGER notNull?
//     ;

// decimalExactNumericType
//     : (DECIMAL | DEC) (LEFT_PAREN precision (COMMA scale)? RIGHT_PAREN
//     notNull?)?
//     ;

// precision
//     : unsignedDecimalInteger
//     ;

// scale
//     : unsignedDecimalInteger
//     ;

// approximateNumericType
//     : FLOAT16 notNull?
//     | FLOAT32 notNull?
//     | FLOAT64 notNull?
//     | FLOAT128 notNull?
//     | FLOAT256 notNull?
//     | FLOAT (LEFT_PAREN precision (COMMA scale)? RIGHT_PAREN)? notNull?
//     | REAL notNull?
//     | DOUBLE PRECISION? notNull?
//     ;

// temporalType
//     : temporalInstantType
//     | temporalDurationType
//     ;

// temporalInstantType
//     : datetimeType
//     | localdatetimeType
//     | dateType
//     | timeType
//     | localtimeType
//     ;

// datetimeType
//     : ZONED DATETIME notNull?
//     | TIMESTAMP WITH TIME ZONE notNull?
//     ;

// localdatetimeType
//     : LOCAL DATETIME notNull?
//     | TIMESTAMP (WITHOUT TIME ZONE)? notNull?
//     ;

// dateType
//     : DATE notNull?
//     ;

// timeType
//     : ZONED TIME notNull?
//     | TIME WITH TIME ZONE notNull?
//     ;

// localtimeType
//     : LOCAL TIME notNull?
//     | TIME WITHOUT TIME ZONE notNull?
//     ;

// temporalDurationType
//     : DURATION LEFT_PAREN temporalDurationQualifier RIGHT_PAREN notNull?
//     ;

// temporalDurationQualifier
//     : YEAR TO MONTH
//     | DAY TO SECOND
//     ;
enum class TemporalDurationQualifier { YearToMonth, DayToSecond };

// fieldType
//    : fieldName typed? valueType
//    ;
struct FieldType : NodeBase<FieldType> {
  FieldName name;
  ValueTypePtr type;
};
GQL_AST_STRUCT(FieldType, name, type)

// fieldTypeList
//    : fieldType (COMMA fieldType)*
//    ;
using FieldTypeList = std::vector<FieldType>;

// fieldTypesSpecification
//    : LEFT_BRACE fieldTypeList? RIGHT_BRACE
//    ;
struct FieldTypesSpecification : NodeBase<FieldTypesSpecification> {
  FieldTypeList fields;
};
GQL_AST_STRUCT(FieldTypesSpecification, fields)

//  bindingTableType
//    : BINDING? TABLE fieldTypesSpecification
//    ;
using BindingTableType = FieldTypesSpecification;

// bindingTableReferenceValueType
//    : bindingTableType notNull?
//    ;
struct BindingTableReferenceValueType {
  BindingTableType type;
  bool notNull = false;
};
GQL_AST_STRUCT(BindingTableReferenceValueType, type, notNull)

// immaterialValueType
//     : nullType
//     | emptyType
//     ;

// nullType
//     : NULL_KW
//     ;

// emptyType
//     : NULL_KW notNull
//     | NOTHING
//     ;

// closedGraphReferenceValueType
//     : PROPERTY? GRAPH nestedGraphTypeSpecification notNull?
//     ;

// openGraphReferenceValueType
//     : ANY PROPERTY? GRAPH notNull?
//     ;

// graphReferenceValueType
//     : openGraphReferenceValueType
//     | closedGraphReferenceValueType
//     ;
struct GraphReferenceValueType : NodeBase<GraphReferenceValueType> {
  std::optional<NestedGraphTypeSpecification> type;  // Not set means "any"
  bool notNull = false;
};
GQL_AST_STRUCT(GraphReferenceValueType, type, notNull)

// bindingTableReferenceValueType
//     : bindingTableType notNull?
//     ;

// closedNodeReferenceValueType
//     : nodeTypeSpecification notNull?
//     ;

// openNodeReferenceValueType
//     : ANY? nodeSynonym notNull?
//     ;

// nodeReferenceValueType
//     : openNodeReferenceValueType
//     | closedNodeReferenceValueType
//     ;
struct NodeReferenceValueType : NodeBase<NodeReferenceValueType> {
  std::optional<NodeTypeSpecification> type;  // Not set means "any"
};
GQL_AST_STRUCT(NodeReferenceValueType, type)

// closedEdgeReferenceValueType
//     : edgeTypeSpecification notNull?
//     ;

// openEdgeReferenceValueType
//     : ANY? edgeSynonym notNull?
//     ;

// edgeReferenceValueType
//     : openEdgeReferenceValueType
//     | closedEdgeReferenceValueType
//     ;
struct EdgeReferenceValueType : NodeBase<EdgeReferenceValueType> {
  std::optional<EdgeTypeSpecification> type;  // Not set means "any"
};
GQL_AST_STRUCT(EdgeReferenceValueType, type)

// referenceValueType
//     : graphReferenceValueType
//     | bindingTableReferenceValueType
//     | nodeReferenceValueType
//     | edgeReferenceValueType
//     ;

// pathValueType
//     : PATH notNull?
//     ;

// listValueTypeName
//     : GROUP? listValueTypeNameSynonym
//     ;

// recordType
//     : ANY? RECORD notNull?
//     | RECORD? fieldTypesSpecification notNull?
//     ;
struct RecordType : NodeBase<RecordType> {
  std::optional<FieldTypesSpecification> fields;  // When not set it means ANY
};
GQL_AST_STRUCT(RecordType, fields)

// predefinedType
//    : booleanType
//    | characterStringType
//    | byteStringType
//    | numericType
//    | temporalType
//    | referenceValueType
//    | immaterialValueType
//    ;

// valueType
//     : predefinedType
//     // <constructed value type>
//     | pathValueType
//     | listValueTypeName LEFT_ANGLE_BRACKET valueType RIGHT_ANGLE_BRACKET
//                (LEFT_BRACKET maxLength RIGHT_BRACKET)? notNull?
//     | valueType listValueTypeName (LEFT_BRACKET maxLength RIGHT_BRACKET)?
//     notNull? | listValueTypeName (LEFT_BRACKET maxLength RIGHT_BRACKET)?
//     notNull? | recordType
//     // <dynamic union type>
//     | ANY VALUE? notNull?
//     | ANY? PROPERTY VALUE notNull?
//     // <closed dynamic union type>
//     | ANY VALUE? LEFT_ANGLE_BRACKET valueType (VERTICAL_BAR valueType)*
//     RIGHT_ANGLE_BRACKET | valueType VERTICAL_BAR valueType
//     ;
struct ValueType : NodeBase<ValueType> {
  struct List {
    bool isGroup = false;
    ValueTypePtr valueType;
    std::optional<UnsignedInteger> maxLength;
  };
  struct Union {
    std::vector<ValueTypePtr> types;  // At least one type
  };

  std::variant<SimplePredefinedType,
               StringType,
               SimpleNumericType,
               BinaryExactNumericType,
               ScaleNumericType,
               GraphReferenceValueType,
               BindingTableReferenceValueType,
               NodeReferenceValueType,
               EdgeReferenceValueType,
               List,
               RecordType,
               Union>
      typeOption;

  bool notNull = false;  // Not applicable for Union
};

GQL_AST_STRUCT(ValueType, typeOption, notNull)
GQL_AST_STRUCT(ValueType::List, valueType, maxLength)
GQL_AST_STRUCT(ValueType::Union, types)

}  // namespace gql::ast
