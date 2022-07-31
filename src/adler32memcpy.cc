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

#include "adler32memcpy.h"
#include <immintrin.h>
#include <memory>

// We are using (a modified form of) adler-32 checksum algorithm instead
// of CRC since adler-32 is faster than CRC.
// (Comparison: http://guru.multimedia.cx/crc32-vs-adler32/)
// This form of adler is bit modified, instead of treating the data in
// units of bytes, 32-bit data is taken as a unit and two 64-bit
// checksums are done (we could have one checksum but two checksums
// make the code run faster).

// Adler-32 implementation:
//   Data is treated as 1-byte numbers and,
//   there are two 16-bit numbers a and b
//   Initialize a with 1 and b with 0.
//   for each data unit 'd'
//      a += d
//      b += a
//   checksum = a<<16 + b
//   This sum should never overflow.
//
// Adler-64+64 implementation:
//   (applied in this code)
//   Data is treated as 32-bit numbers and whole data is separated into two
//   streams, and hence the two checksums a1, a2, b1 and b2.
//   Initialize a1 and a2 with 1, b1 and b2 with 0
//   add first dataunit to a1
//   add a1 to b1
//   add second dataunit to a1
//   add a1 to b1
//   add third dataunit to a2
//   add a2 to b2
//   add fourth dataunit to a2
//   add a2 to b2
//   ...
//   repeat the sequence back for next 4 dataunits
//
//   variable A = XMM6 and variable B = XMM7.
//   (a1 = lower 8 bytes of XMM6 and b1 = lower 8 bytes of XMM7)

// Assumptions
// 1. size_in_bytes is a multiple of 16.
// 2. srcmem and dstmem are 16 byte aligned.
// 3. size_in_bytes is less than 2^19 bytes.

// Assumption 3 ensures that there is no overflow when numbers are being
// added (we can remove this assumption by doing modulus with a prime
// number when it is just about to overflow but that would be a very costly
// exercise)

// Returns string representation of the Adler checksum.
string AdlerChecksum::ToHexString() const {
  char buffer[128];
  snprintf(buffer, sizeof(buffer), "%016llx %016llx %016llx %016llx %016llx %016llx %016llx %016llx", a[0], a[1], a[2], a[3], b[0], b[1], b[2], b[3]);
  return string(buffer);
}

// Calculates Adler checksum for supplied data.
bool CalculateAdlerChecksum(uint64 *__restrict data64, unsigned int size_in_bytes,
                            AdlerChecksum *__restrict checksum) {
  // Use this data wrapper to access memory with 64bit read/write.
  uint64 data;
  unsigned int count = size_in_bytes / sizeof(data);

  if (count > (1U) << 19) {
    // Size is too large, must be strictly less than 512 KB.
    return false;
  }

  AdlerChecksum ret{};

  for(unsigned i = 0; i < count; i += 4)
  {
    ret.increment({data64[i], data64[i + 1], data64[i + 2], data64[i + 3]});
  }

  *checksum = ret;
  return true;
}

// C implementation of Adler memory copy.
bool AdlerMemcpyC(uint64 *__restrict dstmem64, uint64 *__restrict srcmem64,
                  unsigned int size_in_bytes, AdlerChecksum *__restrict checksum) {
    uint64 data;
    unsigned int count = size_in_bytes / sizeof(data);

    if (count > ((1U) << 19)) {
        // Size is too large, must be strictly less than 512 KB.
        return false;
    }

    AdlerChecksum ret{};
    for(unsigned int i = 0; i < count; i += 8)
    {
      const std::array<uint64, 8> buffer = extractData<8>(srcmem64 + i);
      ret.increment({buffer[0], buffer[1], buffer[2], buffer[3]});
      ret.increment({buffer[4], buffer[5], buffer[6], buffer[7]});
      for(int j = 0; j < 8; ++j) {
        __builtin_nontemporal_store(buffer[j], dstmem64 + i + j);
      }
    }
    *checksum = ret;
    return true;
}

// C implementation of Adler memory copy with some float point ops,
// attempting to warm up the CPU.
bool AdlerMemcpyWarmC(uint64 *__restrict dstmem64, uint64 *__restrict srcmem64,
                      unsigned int size_in_bytes, AdlerChecksum *__restrict checksum) {
  return AdlerMemcpyC(dstmem64, srcmem64, size_in_bytes, checksum);
}

// x86_64 SSE2 assembly implementation of fast and stressful Adler memory copy.
bool AdlerMemcpyAsm(uint64 *__restrict dstmem64, uint64 *__restrict srcmem64,
                    unsigned int size_in_bytes, AdlerChecksum *__restrict checksum) {
    return AdlerMemcpyC(dstmem64, srcmem64, size_in_bytes, checksum);
}
