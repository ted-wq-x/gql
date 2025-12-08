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

#include <memory>
#include <vector>

#include "operators.h"

namespace gql_demo {

class SearchOp;
using SearchOpPtr = std::shared_ptr<SearchOp>;

// |OpSequence|s are planning-time helpers to build operators graph.

class IOpSequence {
 public:
  virtual ~IOpSequence() {}
  virtual void InsertSequence(SearchOpPtr firstOp, SearchOpPtr lastOp) = 0;
};

class LinearOpSequence {
 public:
  explicit LinearOpSequence(IOpSequence& opSequence);
  ~LinearOpSequence();

  void Append(SearchOpPtr op);
  void Append(SearchOpPtr firstOp, SearchOpPtr lastOp);
  void Remove(SearchOpPtr op);

 private:
  IOpSequence& opSequence_;
  SearchOpPtr lastOp_;
  std::vector<std::pair<SearchOpPtr, SearchOpPtr>> ops_;
};

class ForkOpSequence : public IOpSequence {
 public:
  ForkOpSequence(LinearOpSequence& opSequence);
  ~ForkOpSequence();

  void InsertSequence(SearchOpPtr firstOp, SearchOpPtr lastOp) override;
  void InsertShortcut();

 private:
  LinearOpSequence& opSequence_;
  std::vector<std::pair<SearchOpPtr, SearchOpPtr>> branches_;
  bool shortcutInserted_ = false;
};

class QuantifiedOpSequence : public IOpSequence {
 public:
  explicit QuantifiedOpSequence(std::shared_ptr<QuantifiedOp> quantifiedOp);

  void InsertSequence(SearchOpPtr firstOp, SearchOpPtr lastOp) override;

 private:
  std::shared_ptr<QuantifiedOp> quantifiedOp_;
};

class StoringOpSequence : public IOpSequence {
 public:
  explicit StoringOpSequence(SearchOpPtr& firstOp) : firstOp_(firstOp) {}

  void InsertSequence(SearchOpPtr firstOp, SearchOpPtr) override {
    firstOp_ = std::move(firstOp);
  }

 private:
  SearchOpPtr& firstOp_;
};

}  // namespace gql_demo