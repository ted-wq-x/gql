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

#include <functional>
#include <stack>
#include <utility>

namespace gql_demo::helpers {

class ScopeExit {
 public:
  ScopeExit() = default;
  ScopeExit(const ScopeExit&) = delete;
  ScopeExit& operator=(const ScopeExit&) = delete;

  ~ScopeExit() noexcept {
    while (!actions_.empty()) {
      actions_.top()();
      actions_.pop();
    }
  }

  template <typename F>
  void Add(F&& f) {
    actions_.emplace(std::forward<F>(f));
  }

  void Clear() {
    while (!actions_.empty()) {
      actions_.pop();
    }
  }

 private:
  std::stack<std::function<void()>> actions_;
};

}  // namespace gql_demo::helpers