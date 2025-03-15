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

#include <cassert>
#include <cerrno>
#include <cstdint>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <libkern/OSCacheControl.h>
#include <limits>
#include <stack>
#include <sys/mman.h>
#include "assembler.hpp"
#include "constants.hpp"

using std::uintmax_t;
using std::fstream;

// The main function takes in arguments and then executes the code.
int main(int argc, char** argv) {
  // We expect most of the time the program is provided.
  if (__builtin_expect(argc < 2, 0)) {
    std::cerr << "zero: please provide the input file" << std::endl;
    return 1;
  }

  // Get the file name, and consequently read the size of the file.
  // This may include comments, but these will just lead to wasted space.
  char* fileName = argv[1];
  uintmax_t fileSize = std::filesystem::file_size(fileName);
  // Perform the memory mapping. This is a pessimistic estimate. We assume that
  // we perform 4 instructions (128 bits) per BF instruction, plus an additional
  // 8 instructions for setup and teardown.
  uintmax_t instructionSize = (INS_WIDTH * fileSize) + (INS_WIDTH * INS_BUFFER);
  // Assert that we can address this, it's an assertion due to unlikeliness.
  assert(instructionSize < std::numeric_limits<uint32_t>::max());
  // Create the call via mmap.
  void* rawAddress = mmap(nullptr,
                          instructionSize,
                          PROT_READ | PROT_WRITE | PROT_EXEC,
                          MAP_PRIVATE | MAP_ANON | MAP_JIT,
                          -1,
                          0);
  if (__builtin_expect(rawAddress == MAP_FAILED, 0)) {
    std::cerr << "zero: could not make executable memory via mmap" << std::endl;
    return 1;
  }
  uint32_t* baseAddress = reinterpret_cast<uint32_t*>(rawAddress);

  // Create the memory and assembler.
  char memory[MEMORY_SIZE] = {0}; // Initialize to zero for compliance.
  // Use new since the constructor/destructor of Assembler take care of some
  // kernel-level things needed to write JIT memory.
  Assembler* assembler = new Assembler(baseAddress, instructionSize);

  // Write the prelude with the assembler.
  assembler->prelude(memory);

  // Read the file line by line and write it to the address space.
  char ch;
  fstream file(fileName, fstream::in);
  while (file >> ch) {
    // TODO: Compiler.
  }

  // Write the postlude with the assembler.
  assembler->postlude();

  // Explicitly delete the assembler to clean up everything.
  delete assembler;

  // Jump to the actual JIT subroutine.
  return reinterpret_cast<uint32_t(*)()>(baseAddress)();
}

