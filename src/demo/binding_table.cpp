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

#include "binding_table.h"

#include <algorithm>
#include <cassert>

namespace gql_demo {

// static
BindingTable BindingTable::Unit() {
  BindingTable table;
  table.tableSize_ = 1;
  return table;
}

BindingTable::Inserter BindingTable::CreateInserter() {
  return {*this};
}

BindingTable::ColumnInserter BindingTable::CreateColumnInserter() {
  return {*this};
}

std::unordered_map<std::string, Value> BindingTable::rowRecord(
    size_t rowIdx) const {
  std::unordered_map<std::string, Value> row;
  for (auto& [colName, colValues] : columns_) {
    row[colName] = colValues[rowIdx];
  }
  return row;
}

void BindingTable::BindingTable::AddColumn(const std::string& name,
                                           AddColumnMode mode) {
  assert(size() == 0);
  auto [_, inserted] = columns_.emplace(name, Column{});
  assert(inserted);
  if (mode == SetPreferredColumnSequence) {
    if (!preferredColumnSequence_) {
      preferredColumnSequence_.emplace();
    }
    preferredColumnSequence_->push_back(name);
  }
}

std::vector<std::string> BindingTable::preferredColumnSequence() const {
  if (preferredColumnSequence_) {
    assert(preferredColumnSequence_->size() == columns_.size());
    return *preferredColumnSequence_;
  }
  std::vector<std::string> result;
  result.reserve(columns_.size());
  for (auto& col : columns_) {
    result.push_back(col.first);
  }
  std::sort(result.begin(), result.end());
  return result;
}

bool BindingTable::AreRowsEqual(size_t row1, size_t row2) const {
  for (auto& [_, colValues] : columns_) {
    if (colValues[row1] != colValues[row2]) {
      return false;
    }
  }
  return true;
}

bool BindingTable::AreRowsEqual(size_t row,
                                const BindingTable& otherTable,
                                size_t otherRow) const {
  assert(columns_.size() == otherTable.columns_.size());
  for (auto& [colName, colValues] : columns_) {
    if (colValues[row] != otherTable.columns_.find(colName)->second[otherRow]) {
      return false;
    }
  }
  return true;
}

void BindingTable::AppendProduct(const BindingTable& row,
                                 size_t rowIdx,
                                 const BindingTable& table,
                                 bool isOptional) {
  size_t appendSize = table.size();
  if (table.size() == 0 && isOptional) {
    appendSize = 1;
  }

  for (auto& col : row.columns_) {
    auto& targetCol = columns_[col.first];
    targetCol.resize(targetCol.size() + appendSize, col.second[rowIdx]);
  }
  for (auto& col : table.columns_) {
    auto& targetCol = columns_[col.first];
    if (table.size() == 0 && isOptional) {
      targetCol.emplace_back(Null{});
    } else {
      targetCol.insert(targetCol.end(), col.second.begin(), col.second.end());
    }
  }
}

void BindingTable::Concat(BindingTable&& src) {
  preferredColumnSequence_.reset();
  for (auto& col : src.columns_) {
    assert(!columns_.count(col.first));
    columns_[col.first] = std::move(col.second);
  }
}

BindingTable BindingTable::Yield(const std::vector<std::string>& colNames,
                                 const WorkingRecord& record) && {
  BindingTable newTable;
  for (auto& col : colNames) {
    if (auto colIt = columns_.find(col); colIt != columns_.end()) {
      newTable.columns_[col] = std::move(colIt->second);
    } else {
      // NOTE 258 - As opposed to the General Rules for <binding variable>,
      // <yield item>s only consider the current working record and ignore the
      // working records of any parent execution contexts that precede the
      // current execution context in the current execution stack.
      newTable.columns_[col] =
          BindingTable::Column(size(), record.fieldValue(col, true));
    }
  }
  return newTable;
}

void BindingTable::KeepDistinctRows() {
  if (columns_.empty())
    return;

  for (size_t rowIdx = 0; rowIdx < size();) {
    bool isDuplicate = false;
    for (size_t i = 0; i < rowIdx; ++i) {
      if (AreRowsEqual(i, rowIdx)) {
        isDuplicate = true;
        break;
      }
    }
    if (isDuplicate) {
      RemoveRow(rowIdx);
    } else {
      rowIdx++;
    }
  }
}

void BindingTable::Slice(size_t offset, size_t limit) {
  for (auto& [_, colValues] : columns_) {
    if (offset >= colValues.size() || limit == 0) {
      colValues.clear();
    } else {
      colValues.erase(colValues.begin(), colValues.begin() + offset);
      if (limit < colValues.size()) {
        colValues.erase(colValues.begin() + limit, colValues.end());
      }
    }
  }
}

void BindingTable::RemoveRow(size_t rowIdx) {
  for (auto& [_, colValues] : columns_) {
    assert(rowIdx < colValues.size());
    colValues.erase(colValues.begin() + rowIdx);
  }
}

BindingTable TableConjunction(const BindingTable& table1,
                              const gql::ast::SetOperator& op,
                              const BindingTable& table2) {
  BindingTable result;
  result.preferredColumnSequence_ = table1.preferredColumnSequence_;
  for (auto& [colName, _] : table1.columns_) {
    result.columns_.emplace(colName, BindingTable::Column{});
  }

  auto appendRowsToResult = [&result](const BindingTable& table, size_t idx,
                                      size_t numCopies) {
    for (auto& [colName, colValues] : table.columns_) {
      auto& dstCol = result.columns_[colName];
      dstCol.resize(dstCol.size() + numCopies, colValues[idx]);
    }
  };

  auto table1Size = table1.size();
  auto table2Size = table2.size();
  std::vector<bool> isDuplicate1(table1Size, false);
  std::vector<bool> isDuplicate2(table2Size, false);
  for (size_t row1Idx = 0; row1Idx < table1Size; row1Idx++) {
    if (isDuplicate1[row1Idx]) {
      continue;
    }
    size_t numDuplicates1 = 1;
    for (size_t i = row1Idx + 1; i < table1Size; i++) {
      if (isDuplicate1[i]) {
        continue;
      }
      if (table1.AreRowsEqual(i, row1Idx)) {
        isDuplicate1[i] = true;
        numDuplicates1++;
      }
    }
    size_t numDuplicates2 = 0;
    for (size_t row2Idx = 0; row2Idx < table2Size; row2Idx++) {
      if (isDuplicate2[row2Idx]) {
        continue;
      }
      if (table1.AreRowsEqual(row1Idx, table2, row2Idx)) {
        isDuplicate2[row2Idx] = true;
        numDuplicates2++;
      }
    }
    size_t rowsToAppend = 0;
    switch (op.kind) {
      case gql::ast::SetOperator::Kind::UNION:
        if (op.quantifier == gql::ast::SetQuantifier::DISTINCT) {
          rowsToAppend = 1;
        } else {
          rowsToAppend = numDuplicates1 + numDuplicates2;
        }
        break;
      case gql::ast::SetOperator::Kind::EXCEPT:
        if (op.quantifier == gql::ast::SetQuantifier::DISTINCT) {
          rowsToAppend = (numDuplicates1 > 0 && numDuplicates2 == 0) ? 1 : 0;
        } else {
          rowsToAppend = numDuplicates1 > numDuplicates2
                             ? (numDuplicates1 - numDuplicates2)
                             : 0;
        }
        break;
      case gql::ast::SetOperator::Kind::INTERSECT:
        if (op.quantifier == gql::ast::SetQuantifier::DISTINCT) {
          rowsToAppend = (numDuplicates1 > 0 && numDuplicates2 > 0) ? 1 : 0;
        } else {
          rowsToAppend = std::min(numDuplicates1, numDuplicates2);
        }
        break;
    }
    if (rowsToAppend > 0) {
      appendRowsToResult(table1, row1Idx, rowsToAppend);
    }
  }
  if (op.kind == gql::ast::SetOperator::Kind::UNION) {
    // Process rows in the second table that have no duplicates in the first
    // table.
    for (size_t row2Idx = 0; row2Idx < table2Size; row2Idx++) {
      if (isDuplicate2[row2Idx]) {
        continue;
      }
      appendRowsToResult(table2, row2Idx, 1);
      if (op.quantifier == gql::ast::SetQuantifier::DISTINCT) {
        for (size_t i = row2Idx + 1; i < table2Size; i++) {
          if (isDuplicate2[i]) {
            continue;
          }
          if (table2.AreRowsEqual(i, row2Idx)) {
            isDuplicate2[i] = true;
          }
        }
      }
    }
  }
  return result;
}

BindingTable::Inserter::Inserter(BindingTable& table)
    : table_(table), rowIndex_(table.size()) {}

void BindingTable::Inserter::Reserve(size_t rowCount) {
  for (auto& col : table_.columns_) {
    assert(col.second.empty());
    col.second.reserve(col.second.size() + rowCount);
  }
}

void BindingTable::Inserter::NextRow() {
  rowIndex_++;
  if (table_.columns_.empty()) {
    table_.tableSize_++;
  }
}

Value& BindingTable::Inserter::Emplace(const std::string& name) {
  auto colIt = table_.columns_.find(name);
  assert(colIt != table_.columns_.end());
  auto& col = colIt->second;
  assert(col.size() == rowIndex_);
  return col.emplace_back();
}

void BindingTable::Inserter::Set(const std::string& name, const Value& value) {
  Emplace(name) = value;
}

Value& BindingTable::ColumnInserter::Emplace(const std::string& name) {
  if (rowIndex_ == 0) {
    auto [colIt, inserted] = table_.columns_.emplace(name, Column{});
    assert(inserted);
    if (reserve_) {
      colIt->second.reserve(*reserve_);
    }
  }
  return Inserter::Emplace(name);
}

const Value& WorkingRecord::fieldValue(const std::string& name,
                                       bool ignoreParentRecord) const {
  if (auto* value = hasField(name, ignoreParentRecord)) {
    return *value;
  }
  assert(false);
  static Value kNull{Null{}};
  return kNull;
}

const Value* WorkingRecord::hasField(const std::string& name,
                                     bool ignoreParentRecord) const {
  static Value kNull{Null{}};
  if (filter_ && !filter_->count(name)) {
    return nullptr;
  }

  if (table) {
    auto colIt = table->columns_.find(name);
    if (colIt != table->columns_.end()) {
      return &colIt->second[index];
    }
  }
  if (!ignoreParentRecord && parentRecord) {
    return parentRecord->hasField(name);
  }
  return nullptr;
}

}  // namespace gql_demo