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

namespace gql::ast {

template <typename T>
class value_ptr {
 public:
  value_ptr() = default;
  value_ptr(const value_ptr& other) { *this = other; }
  value_ptr(value_ptr&& other) noexcept : ptr_(std::move(other.ptr_)) {}

  value_ptr& operator=(const value_ptr& other) {
    if (!other.ptr_) {
      ptr_.reset();
    } else {
      *this = *other.ptr_;
    }
    return *this;
  }

  value_ptr& operator=(value_ptr&& other) noexcept {
    ptr_ = std::move(other.ptr_);
    return *this;
  }

  value_ptr& operator=(const T& other) {
    if (!ptr_) {
      ptr_ = std::make_unique<T>(other);
    } else {
      *ptr_ = other;
    }
    return *this;
  }

  bool operator==(const value_ptr& other) const {
    ensure_created();
    return *ptr_ == *other;
  }

  bool operator!=(const value_ptr& other) const { return !(*this == other); }

  T* operator->() noexcept { return get(); }
  const T* operator->() const noexcept { return get(); }

  T& operator*() noexcept { return *get(); }
  const T& operator*() const noexcept { return *get(); }

  T* get() noexcept {
    ensure_created();
    return ptr_.get();
  }
  const T* get() const noexcept {
    ensure_created();
    return ptr_.get();
  }

 private:
  void ensure_created() const {
    if (!ptr_) {
      ptr_ = std::make_unique<T>();
    }
  }

  mutable std::unique_ptr<T> ptr_;
};

}  // namespace gql::ast
