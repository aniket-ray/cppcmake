// Copyright 2011 Google Inc. All Rights Reserved.
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

#ifndef NINJA_MAP_H_
#define NINJA_MAP_H_

#include <string.h>
#include <algorithm>
#include "string_piece.h"
#include "util.h"

// MurmurHash2, by Austin Appleby
static inline unsigned int MurmurHash2(const void* key, size_t len) {
  static const unsigned int seed = 0xDECAFBAD;
  const unsigned int m = 0x5bd1e995;
  const int r = 24;
  unsigned int h = seed ^ len;
  const unsigned char* data = (const unsigned char*)key;
  while (len >= 4) {
    unsigned int k;
    memcpy(&k, data, sizeof k);
    k *= m;
    k ^= k >> r;
    k *= m;
    h *= m;
    h ^= k;
    data += 4;
    len -= 4;
  }
  switch (len) {
    case 3:
      h ^= data[2] << 16;
      NINJA_FALLTHROUGH;
    case 2:
      h ^= data[1] << 8;
      NINJA_FALLTHROUGH;
    case 1:
      h ^= data[0];
      h *= m;
  };
  h ^= h >> 13;
  h *= m;
  h ^= h >> 15;
  return h;
}

#include <unordered_map>

namespace std {
template <>
struct hash<StringPiece> {
  typedef StringPiece argument_type;
  typedef size_t result_type;

  size_t operator()(StringPiece key) const { return MurmurHash2(key.str_, key.len_); }
};
}  // namespace std

/// A template for hash_maps keyed by a StringPiece whose string is
/// owned externally (typically by the values).  Use like:
/// ExternalStringHash<Foo*>::Type foos; to make foos into a hash
/// mapping StringPiece => Foo*.
template <typename V>
struct ExternalStringHashMap {
  typedef std::unordered_map<StringPiece, V> Type;
};

#endif  // NINJA_MAP_H_
