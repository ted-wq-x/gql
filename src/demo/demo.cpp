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

#include "demo.h"

namespace gql_demo {

#ifndef NDEBUG
// static
void Demo::ContextStateChecker::CheckTableType(
    const BindingTable& table,
    const gql::ast::FieldTypeList& tableType) {
  std::unordered_set<std::string> expected, actual;
  for (auto& f : tableType) {
    expected.insert(f.name.name);
  }
  for (auto& [colName, _] : table.columns()) {
    actual.insert(colName);
  }
  assert(expected == actual);
  // TODO: Check group/element columns.
}

// static
void Demo::ContextStateChecker::CheckRecordType(
    const WorkingRecord& record,
    const gql::ast::FieldTypeList& recordType) {
  for (auto& f : recordType) {
    assert(record.hasField(f.name.name));
    // TODO: Check field type.
  }
}

Demo::ContextStateChecker::ContextStateChecker(
    const gql::syntax_analyzer::NodeBase& node,
    const ExecutionContext& context)
    : node_(node), context_(context) {}

Demo::ContextStateChecker::~ContextStateChecker() {
  if (node_.debugOutgoingWorkingRecordType) {
    CheckRecordType(context_.workingRecord,
                    **node_.debugOutgoingWorkingRecordType);
  }
  if (node_.debugTableType) {
    CheckTableType(context_.workingTable, **node_.debugTableType);
  }
}
#endif

}  // namespace gql_demo
