// Copyright 2017 Google Inc. All Rights Reserved.
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

#ifndef CACHE_ALIGNED_H_
#define CACHE_ALIGNED_H_

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>  // memcpy
#include <algorithm>
#include <memory>
#include <new>

#include "arch_specific.h"
#include "compiler_specific.h"
#include "simd/simd.h"
#include "status.h"

namespace pik {

// Functions that depend on the cache line size.
class CacheAligned {
 public:
  static constexpr size_t kPointerSize = sizeof(void*);
  static constexpr size_t kCacheLineSize = 64;

  static void* Allocate(const size_t bytes) {
    PIK_ASSERT(bytes < 1ULL << 63);
    char* const allocated = static_cast<char*>(malloc(bytes + kCacheLineSize));
    if (allocated == nullptr) {
      return nullptr;
    }
    const uintptr_t misalignment =
        reinterpret_cast<uintptr_t>(allocated) & (kCacheLineSize - 1);
    // malloc is at least kPointerSize aligned, so we can store the "allocated"
    // pointer immediately before the aligned memory.
    PIK_ASSERT(misalignment % kPointerSize == 0);
    char* const aligned = allocated + kCacheLineSize - misalignment;
    memcpy(aligned - kPointerSize, &allocated, kPointerSize);
    return aligned;
  }

  // Template allows freeing pointer-to-const.
  template <typename T>
  static void Free(T* aligned_pointer) {
    if (aligned_pointer == nullptr) {
      return;
    }
    const char* const aligned = reinterpret_cast<const char*>(aligned_pointer);
    PIK_ASSERT(reinterpret_cast<uintptr_t>(aligned) % kCacheLineSize == 0);
    char* allocated;
    memcpy(&allocated, aligned - kPointerSize, kPointerSize);
    PIK_ASSERT(allocated <= aligned - kPointerSize);
    PIK_ASSERT(allocated >= aligned - kCacheLineSize);
    free(allocated);
  }

  // Overwrites "to_items" without loading it into cache (read-for-ownership).
  // Copies kCacheLineSize bytes from/to naturally aligned addresses.
  template <typename T>
  static void StreamCacheLine(const T* PIK_RESTRICT from, T* PIK_RESTRICT to) {
    static_assert(16 % sizeof(T) == 0, "T must fit in a lane");
    const SIMD_NAMESPACE::Part<T, 16 / sizeof(T), SIMD_TARGET> d;
    PIK_COMPILER_FENCE;
    const auto v0 = load(d, from + 0);
    const auto v1 = load(d, from + 1);
    const auto v2 = load(d, from + 2);
    const auto v3 = load(d, from + 3);
    // Fences prevent the compiler from reordering loads/stores, which may
    // interfere with write-combining.
    PIK_COMPILER_FENCE;
    stream(to + 0, d, v0);
    stream(to + 1, d, v1);
    stream(to + 2, d, v2);
    stream(to + 3, d, v3);
    PIK_COMPILER_FENCE;
  }
};

template <typename T>
using CacheAlignedUniquePtrT = std::unique_ptr<T, void (*)(T*)>;

using CacheAlignedUniquePtr = CacheAlignedUniquePtrT<uint8_t>;

template <typename T>
inline void DestroyAndAlignedFree(T* t) {
  t->~T();
  CacheAligned::Free(t);
}

template <typename T, typename... Args>
inline CacheAlignedUniquePtrT<T> Allocate(Args&&... args) {
  void* mem = CacheAligned::Allocate(sizeof(T));
  T* t = new (mem) T(std::forward<Args>(args)...);
  return CacheAlignedUniquePtrT<T>(t, &DestroyAndAlignedFree<T>);
}

// Does not invoke constructors.
template <typename T = uint8_t>
inline CacheAlignedUniquePtrT<T> AllocateArray(const size_t entries) {
  return CacheAlignedUniquePtrT<T>(
      static_cast<T*>(CacheAligned::Allocate(entries * sizeof(T))),
      CacheAligned::Free);
}

}  // namespace pik

#endif  // CACHE_ALIGNED_H_
