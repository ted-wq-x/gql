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

namespace gql::ast {

inline bool operator==(const ValueType& lhs, const ValueType& rhs);

inline bool operator==(const BinaryExactNumericType& lhs,
                       const BinaryExactNumericType& rhs) {
  return lhs.isSigned == rhs.isSigned && lhs.precision == rhs.precision;
}

inline bool operator==(const ScaleNumericType& lhs,
                       const ScaleNumericType& rhs) {
  return lhs.type == rhs.type && lhs.precision == rhs.precision &&
         lhs.scale == rhs.scale;
}

inline bool operator==(const StringType& lhs, const StringType& rhs) {
  return lhs.kind == rhs.kind && lhs.minLength == rhs.minLength &&
         lhs.maxLength == rhs.maxLength;
}

inline bool operator==(const FieldType& lhs, const FieldType& rhs) {
  return lhs.name.name == rhs.name.name && *lhs.type == *rhs.type;
}

inline bool operator==(const FieldTypesSpecification& lhs,
                       const FieldTypesSpecification& rhs) {
  return lhs.fields == rhs.fields;
}

inline bool operator==(const BindingTableReferenceValueType& lhs,
                       const BindingTableReferenceValueType& rhs) {
  return lhs.type == rhs.type && lhs.notNull == rhs.notNull;
}

inline bool operator==(const GraphReferenceValueType& lhs,
                       const GraphReferenceValueType& rhs) {
  // We always consider closed graph types to be different to not implement
  // equality.
  return !lhs.type && !rhs.type && lhs.notNull == rhs.notNull;
}

inline bool operator==(const NodeReferenceValueType& lhs,
                       const NodeReferenceValueType& rhs) {
  // We always consider closed element types to be different to not
  // implement equality.
  return !lhs.type && !rhs.type;
}

inline bool operator==(const EdgeReferenceValueType& lhs,
                       const EdgeReferenceValueType& rhs) {
  // We always consider closed element types to be different to not
  // implement equality.
  return !lhs.type && !rhs.type;
}

inline bool operator==(const RecordType& lhs, const RecordType& rhs) {
  return lhs.fields == rhs.fields;
}

inline bool operator==(const ValueType::List& lhs, const ValueType::List& rhs) {
  return lhs.isGroup == rhs.isGroup && lhs.valueType == rhs.valueType &&
         lhs.maxLength == rhs.maxLength;
}

inline bool operator==(const ValueType::Union& lhs,
                       const ValueType::Union& rhs) {
  return lhs.types == rhs.types;
}

inline bool operator==(const ValueType& lhs, const ValueType& rhs) {
  return lhs.typeOption == rhs.typeOption && lhs.notNull == rhs.notNull;
}

inline bool operator!=(const ValueType& lhs, const ValueType& rhs) {
  return !(lhs == rhs);
}

}  // namespace gql::ast