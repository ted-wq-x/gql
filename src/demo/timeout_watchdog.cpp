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

#include "timeout_watchdog.h"

#include <format>

namespace gql_demo {

TimeoutWatchdog::TimeoutWatchdog(uint64_t timeoutMilliseconds)
    : timeoutMilliseconds_(timeoutMilliseconds),
      startTime_(std::chrono::steady_clock::now()) {}

void TimeoutWatchdog::Check() {
  auto passed = std::chrono::steady_clock::now() - startTime_;
  if (passed > std::chrono::milliseconds(timeoutMilliseconds_)) {
    throw TimeoutException(
        std::chrono::duration_cast<std::chrono::milliseconds>(passed));
  }
}

TimeoutException::TimeoutException(std::chrono::milliseconds ms)
    : runtime_error("") {
  message_ = std::format("GQL request timed out after {0} ms", ms.count());
}

void TimeoutException::SetCurrentTableSize(size_t size) {
  message_ += std::format(". Current table size: {0} rows", size);
}

}  // namespace gql_demo