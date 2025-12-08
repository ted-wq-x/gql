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
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "gql/ast/nodes/statements.h"
#include "value.h"

namespace gql_demo {

class WorkingRecord;

enum AddColumnMode {
  DontSetPreferredColumnSequence,
  SetPreferredColumnSequence
};

class BindingTable {
 public:
  class Inserter;
  class ColumnInserter;
  using Column = std::vector<Value>;

  static BindingTable Unit();

  bool operator==(const BindingTable& other) const = default;

  Inserter CreateInserter();
  ColumnInserter CreateColumnInserter();
  void AddColumn(const std::string& name,
                 AddColumnMode = DontSetPreferredColumnSequence);

  const std::unordered_map<std::string, Column>& columns() const;
  size_t size() const;
  std::unordered_map<std::string, Value> rowRecord(size_t) const;
  std::vector<std::string> preferredColumnSequence() const;

  // Input:   this             row          table
  //          A | B | C | D    A | B        C | D
  //          .............    1 | 2        3 | 4
  //                                        5 | 6
  // Output:  A | B | C | D
  //          .............
  //          1 | 2 | 3 | 4
  //          1 | 2 | 5 | 6
  void AppendProduct(const BindingTable& row,
                     size_t rowIdx,
                     const BindingTable& table,
                     bool isOptional = false);
  // Input:   this         inputTable
  //          A | B        C | D
  //          1 | 2        5 | 6
  //          3 | 4        7 | 8
  // Output:  A | B | C | D
  //          1 | 2 | 5 | 6
  //		  3 | 4 | 7 | 8
  void Concat(BindingTable&&);
  // Input:   this         WorkingRecord    columns
  //          A | B | C    D  | E  | F      [B, D, A, F]
  //          1 | 2 | 3    10 | 11 | 12
  //          4 | 5 | 6
  // Output:  B  | D  | A  | F
  //          2  | 10 | 1  | 12
  //          5  | 10 | 4  | 12
  BindingTable Yield(const std::vector<std::string>& columns,
                     const WorkingRecord&) &&;
  void RemoveRow(size_t rowIdx);
  void KeepDistinctRows();
  void Slice(size_t offset, size_t limit);

 private:
  // tableSize_ is used while there are no columns. Column count can't become
  // zero after it was non-zero thus we can keep value forever.
  size_t tableSize_ = 0;
  std::unordered_map<std::string, Column> columns_;
  std::optional<std::vector<std::string>> preferredColumnSequence_;

  bool AreRowsEqual(size_t row1, size_t row2) const;
  bool AreRowsEqual(size_t row,
                    const BindingTable& otherTable,
                    size_t otherRow) const;

  friend class Inserter;
  friend class ColumnInserter;
  friend class WorkingRecord;
  friend BindingTable TableConjunction(const BindingTable& table1,
                                       const gql::ast::SetOperator&,
                                       const BindingTable& table2);
};

BindingTable TableConjunction(const BindingTable& table1,
                              const gql::ast::SetOperator&,
                              const BindingTable& table2);

class WorkingRecord {
 public:
  const Value& fieldValue(const std::string&,
                          bool ignoreParentRecord = false) const;
  const Value* hasField(const std::string&,
                        bool ignoreParentRecord = false) const;
  void SetFilter(std::unordered_set<std::string> filter) {
    filter_ = std::move(filter);
  }

  const WorkingRecord* parentRecord = nullptr;
  const BindingTable* table = nullptr;
  size_t index = 0;

 private:
  std::optional<std::unordered_set<std::string>> filter_;
};

class BindingTable::Inserter {
 public:
  Inserter(BindingTable& table);

  void Reserve(size_t rowCount);
  void NextRow();
  Value& Emplace(const std::string& name);
  void Set(const std::string& name, const Value& value);

 protected:
  BindingTable& table_;
  size_t rowIndex_ = 0;
};

class BindingTable::ColumnInserter : private Inserter {
 public:
  ColumnInserter(BindingTable& table) : Inserter(table) {}

  using Inserter::NextRow;
  void Reserve(size_t rowCount) { reserve_ = rowCount; }
  Value& Emplace(const std::string& name);
  void Set(const std::string& name, const Value& value) {
    Emplace(name) = value;
  }

 private:
  std::optional<size_t> reserve_;
};

inline const std::unordered_map<std::string, BindingTable::Column>&
BindingTable::columns() const {
  return columns_;
}

inline size_t BindingTable::size() const {
  return columns_.empty() ? tableSize_ : columns_.begin()->second.size();
}

}  // namespace gql_demo
