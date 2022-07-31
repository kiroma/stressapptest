// Copyright 2008 Google Inc. All Rights Reserved.

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//      http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef STRESSAPPTEST_ADLER32MEMCPY_H_
#define STRESSAPPTEST_ADLER32MEMCPY_H_

#include <string>
#include <array>
#include "sattypes.h"

// Encapsulation for Adler checksum. Please see adler32memcpy.cc for more
// detail on the adler checksum algorithm.
struct AdlerChecksum {
  bool operator==(const AdlerChecksum& other) const
  {
    return a == other.a && b == other.b;
  }
  bool operator!=(const AdlerChecksum& other) const
  {
    return a != other.a || b != other.b;
  }
  // Returns string representation of the Adler checksum
  std::string ToHexString() const;
  // Components of Adler checksum.
  std::array<uint64, 2> a {1, 1};
  std::array<uint64, 2> b {0, 0};
};

// Calculates Adler checksum for supplied data.
bool CalculateAdlerChecksum(uint64 *data64, unsigned int size_in_bytes,
                            AdlerChecksum *checksum);

// C implementation of Adler memory copy.
bool AdlerMemcpyC(uint64 *dstmem64, uint64 *srcmem64,
                    unsigned int size_in_bytes, AdlerChecksum *checksum);

// C implementation of Adler memory copy with some float point ops,
// attempting to warm up the CPU.
bool AdlerMemcpyWarmC(uint64 *dstmem64, uint64 *srcmem64,
                      unsigned int size_in_bytes, AdlerChecksum *checksum);

// x86_64 SSE2 assembly implementation of fast and stressful Adler memory copy.
bool AdlerMemcpyAsm(uint64 *dstmem64, uint64 *srcmem64,
                    unsigned int size_in_bytes, AdlerChecksum *checksum);


#endif  // STRESSAPPTEST_ADLER32MEMCPY_H_
