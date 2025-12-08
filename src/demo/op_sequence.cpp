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

#include "op_sequence.h"

namespace gql_demo {

LinearOpSequence::LinearOpSequence(IOpSequence& opSequence)
    : opSequence_(opSequence) {}

LinearOpSequence::~LinearOpSequence() {
  if (ops_.empty()) {
    return;
  }

  auto it = ops_.begin();
  auto prevIt = it;
  for (it++; it != ops_.end(); it++, prevIt++) {
    prevIt->second->SetNextOp(it->first);
  }
  opSequence_.InsertSequence(ops_.front().first, ops_.back().second);
}

void LinearOpSequence::Append(SearchOpPtr op) {
  ops_.emplace_back(op, op);
}

void LinearOpSequence::Append(SearchOpPtr firstOp, SearchOpPtr lastOp) {
  ops_.emplace_back(firstOp, lastOp);
}

void LinearOpSequence::Remove(SearchOpPtr op) {
  std::erase_if(ops_, [&](const auto& opPair) { return opPair.first == op; });
}

ForkOpSequence::ForkOpSequence(LinearOpSequence& opSequence)
    : opSequence_(opSequence) {}

ForkOpSequence::~ForkOpSequence() {
  if (branches_.size() == 0) {
    return;
  }
  if (branches_.size() == 1 && !shortcutInserted_) {
    opSequence_.Append(branches_.front().first, branches_.front().second);
    return;
  }
  auto forkOp = std::make_shared<ForkOp>();
  auto joinOp = std::make_shared<NoOp>();
  for (auto& [firstOp, lastOp] : branches_) {
    forkOp->nextOps.push_back(std::move(firstOp));
    lastOp->SetNextOp(joinOp);
  }
  if (shortcutInserted_) {
    forkOp->SetNextOp(joinOp);
  }
  opSequence_.Append(std::move(forkOp), std::move(joinOp));
}

void ForkOpSequence::InsertSequence(SearchOpPtr firstOp, SearchOpPtr lastOp) {
  branches_.emplace_back(firstOp, lastOp);
}

void ForkOpSequence::InsertShortcut() {
  shortcutInserted_ = true;
}

QuantifiedOpSequence::QuantifiedOpSequence(
    std::shared_ptr<QuantifiedOp> quantifiedOp)
    : quantifiedOp_(std::move(quantifiedOp)) {}

void QuantifiedOpSequence::InsertSequence(SearchOpPtr firstOp,
                                          SearchOpPtr lastOp) {
  quantifiedOp_->loopOp = std::move(firstOp);
  lastOp->SetNextOp(std::make_shared<GoToOp>(quantifiedOp_));
}

}  // namespace gql_demo