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

#include <vector>

namespace gql_demo::helpers {

// DirectionalIterator iterates over indices of a vector in normal or
// reversed order determined at construction time.
// Note: this is not exactly a C++ iterator.
class DirectionalIterator {
 public:
  template <typename T>
  DirectionalIterator(const std::vector<T>& vec, bool reversed)
      : size_(vec.size()), reversed_(reversed) {}

  operator bool() const { return index_ < size_; }
  size_t operator*() const {
    if (reversed_)
      return size_ - index_ - 1;
    return index_;
  }
  void operator++(int) { index_++; }

 private:
  const size_t size_;
  const bool reversed_;
  size_t index_ = 0;
};

}  // namespace gql_demo::helpers