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

#include <chrono>
#include <stdexcept>

namespace gql_demo {

class TimeoutWatchdog {
 public:
  TimeoutWatchdog(uint64_t timeoutMilliseconds);
  void Check();

 private:
  const uint64_t timeoutMilliseconds_;
  const std::chrono::steady_clock::time_point startTime_;
};

class TimeoutException : public std::runtime_error {
 public:
  TimeoutException(std::chrono::milliseconds);
  const char* what() const noexcept override { return message_.c_str(); }
  void SetCurrentTableSize(size_t size);

 private:
  std::string message_;
};

}  // namespace gql_demo