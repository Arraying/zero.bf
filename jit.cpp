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

#include <cerrno>
#include <cstdint>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <libkern/OSCacheControl.h>
#include <stack>
#include <stdexcept>
#include <sys/mman.h>

// The number of bytes an instruction is wide. AArch64 is 32-bit.
#define INS_WIDTH 4

using std::uintmax_t;
using std::fstream;
using std::stack;

// We work with a 50k memory cell, could be more, could be less.
// Wrap-around is forbidden, so it's good to have a large space.
constexpr size_t MEMORY_SIZE = 50000;

// Compile a single instruction.
// This will take advantage of the compilation state, and if applicable
// patch previous compilations.
void compile(uint32_t* baseAddress, 
             size_t &pc,
             char &c,
             stack<size_t> &jumpPoints); 

// Creates an executable memory region sufficiently large to contain all the
// instructions in the program.
uint32_t* createExecutableMemory(uintmax_t length);

// Sets up the stack/registers.
void prelude(uint32_t* baseAddress, size_t &pc, char (&memory)[MEMORY_SIZE]);

// Restores the stack and returns execution.
void postlude(uint32_t* baseAddress, uintmax_t instructionSize, size_t &pc);

// Helper function to write a 32-bit instruction to the byte array.
void writeInstruction(uint32_t* baseAddress, size_t &pc, uint32_t instruction);

// The main function takes in arguments and then executes the code.
int main(int argc, char** argv) {
  // We expect most of the time the program is provided.
  if (__builtin_expect(argc < 2, 0)) {
    std::cerr << "zero: please provide the input file" << std::endl;
    return 1;
  }
  // Get the file name, and consequently read the size of the file.
  // In the JIT version, we do not accept non-code characters in order to be
  // able to very quickly estimate the instruction file size.
  char* fileName = argv[1];
  uintmax_t fileSize = std::filesystem::file_size(fileName);
  // Perform the memory mapping. This is a pessimistic estimate. We assume that
  // we perform 4 instructions (128 bits) per BF instruction, plus an additional
  // 8 instructions for setup and teardown.
  uintmax_t instructionSize = (INS_WIDTH * 4 * fileSize) + (INS_WIDTH * 8);
  uint32_t* baseAddress = createExecutableMemory(instructionSize);
  // Keep track of the program address and memory.
  size_t pc = 0;
  char memory[MEMORY_SIZE] = {0};
  // Write the assembly prelude.
  prelude(baseAddress, pc, memory);
  // Keep track of the jump points for loops.
  stack<size_t> jumpPoints;
  char ch;
  // Read the file line by line and write it to the address space.
  fstream file(fileName, fstream::in);
  while (file >> ch) {
    compile(baseAddress, pc, ch, jumpPoints);
    // Offset by one instruction.
    //pc += INS_WIDTH;
  }
  if (!__builtin_expect(jumpPoints.empty(), 1)) {
    throw std::runtime_error("program parse error: expected ]");
  }
  // Write the assembly postlude.
  postlude(baseAddress, instructionSize, pc);
  // Jump to the actual JIT subroutine.
  return reinterpret_cast<uint32_t(*)()>(baseAddress)();
}

// Actually perform the compilation. This should modify the program counter.
// The register allocation is as follows:
// x19 -- base address of the memory cells [callee saved].
// x20 -- memory index [callee saved].
inline void compile(uint32_t* baseAddress, 
                    size_t &pc,
                    char &c,
                    stack<size_t> &jumpPoints) {
  //std::cout << c << std::endl;
}

// The kernel will automatically unmap this in program exit.
// This positively influences the runtime of the compiled binary.
inline uint32_t* createExecutableMemory(uintmax_t length) {
  void* startingAddress = mmap(nullptr,
                               length,
                               PROT_READ | PROT_WRITE | PROT_EXEC,
                               MAP_PRIVATE | MAP_ANON | MAP_JIT,
                               -1,
                               0);
  return reinterpret_cast<uint32_t*>(startingAddress);
}

inline void prelude(uint32_t* baseAddress,
                  size_t &pc,
                  char (&memory)[MEMORY_SIZE]) {
  // Need to give permission to write to the memory.
  pthread_jit_write_protect_np(0);
  writeInstruction(baseAddress, pc, 0b11010110010111110000001111000000);
  return;
  // Start by snapshotting callee saved registers to the stack:
  // stp x19, x20, [sp, #-16]!
  writeInstruction(baseAddress, pc, 0xf353bfa9);
  // mov x20, #0
  writeInstruction(baseAddress, pc, 0x140080d2);
  // mov x0, xzr
  writeInstruction(baseAddress, pc, 0xe0031faa);
}

inline void postlude(uint32_t* baseAddress,
                     uintmax_t instructionSize,
                     size_t &pc) {
  // Restore the callee saved registers.
  // ldp x19, x20, [sp], #16
  //writeInstruction(baseAddress, pc, 0xf353c1a8);
  // ret
  //writeInstruction(baseAddress, pc, 0xc0035fd6);
  // Disallow writing to the memory region again.
  pthread_jit_write_protect_np(1);
  // Flush the instruction cache explicitly.
  sys_icache_invalidate(baseAddress, instructionSize);
  char* charAddress = reinterpret_cast<char*>(baseAddress);
  __builtin___clear_cache(charAddress, charAddress + instructionSize);
}

inline void writeInstruction(uint32_t* baseAddress,
                             size_t &pc, 
                             uint32_t instruction) {
  baseAddress[pc] = instruction;
  pc++;
}
