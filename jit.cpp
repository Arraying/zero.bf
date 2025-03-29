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
#include <sys/mman.h>
#include "assembler.hpp"
#include "constants.hpp"
#include "compiler.hpp"

using std::uintmax_t;
using std::fstream;

// The main function takes in arguments and then executes the code.
int main(int argc, char** argv) {
  // We expect most of the time the program is provided.
  if (__builtin_expect(argc < 2, false)) {
    std::cerr << "zero: please provide the input file" << std::endl;
    return 1;
  }

  // Get the file name, and consequently read the size of the file.
  // This may include comments, but these will just lead to wasted space.
  char* fileName = argv[1];
  uintmax_t fileSize = std::filesystem::file_size(fileName);
  // Perform a heuristic estimation of how many instructions we will need.
  // Estimate 16 bytes per instruction; this is quite an overestimation.
  uintmax_t heuristic = fileSize << 4;

  // Create the memory and assembler.
  uint8_t memory[MEMORY_SIZE] = {0}; // Initialize to zero for compliance.
  // Use new since the constructor/destructor of Assembler take care of some
  // kernel-level things needed to write JIT memory.
  Assembler assembler(heuristic);

  // Write the prelude with the assembler.
  assembler.prelude();

  // Read the file line by line and write it to the address space.
  char ch;
  fstream file(fileName, fstream::in);

  // Compile it via the compiler.
  Compiler compiler(&assembler);
  while (file >> ch) {
    compiler.compile(ch);
  }

  // Write the postlude with the assembler.
  assembler.postlude();

  // Put everything into executable memory.
  void* baseAddress = assembler.assemble();
  if (__builtin_expect(baseAddress == nullptr, false)) {
    std::cerr << "zero: could not JIT memory region" << std::endl;
    return 1;
  }

  // Jump to the actual JIT subroutine.
  return reinterpret_cast<int(*)(void*)>(baseAddress)(memory);
}

