/*
 * zero.bf, a Brainfuck JIT compiler and interpreter.
 * Copyright (C) 2025 Paul Hübner
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <algorithm>
#include <cassert>
#include <sys/mman.h>
#include <pthread.h>
#include "assembler.hpp"

Assembler::Assembler(uintmax_t heuristic) {
  // Reserve the heuristic so we don't have to alloc every time we write.
  _instructions.reserve(heuristic);

}

void* Assembler::assemble() {
  // Create some executable memory.
  // Every instruction is 4 bytes.
  uint64_t numBytes = _instructions.size() * sizeof(uint32_t);
  void* rawAddress = mmap(nullptr,
                          numBytes,
                          PROT_READ | PROT_WRITE | PROT_EXEC,
                          MAP_PRIVATE | MAP_ANON | MAP_JIT,
                          -1,
                          0);
  if (__builtin_expect(rawAddress == MAP_FAILED, false)) {
    return nullptr;
  }
  // Allow JIT writing.
  pthread_jit_write_protect_np(0);
  // Should be slightly faster than memcpy.
  std::copy(_instructions.begin(), _instructions.end(), reinterpret_cast<uint32_t*>(rawAddress));
  // Disallow JIT writing again.
  pthread_jit_write_protect_np(1);
  // Clear instruction cache.
  char* charAddress = reinterpret_cast<char*>(rawAddress);
  __builtin___clear_cache(charAddress, charAddress + numBytes);
  return rawAddress;
}

