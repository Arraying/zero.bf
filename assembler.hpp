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

#ifndef assembler_hpp
#define assembler_hpp

#include <cassert>
#include <cstdint>
#include <pthread.h>
#include <vector>
#include "constants.hpp"
#include "register.hpp"

class Assembler {
private:
  std::vector<uint32_t> _instructions;

  inline void writeNext(uint32_t instr) {
    _instructions.push_back(instr); 
  }

public:
  Assembler(uintmax_t heuristic);
  void* assemble();

  // The code that gets executed at the beginning of the subroutine.
  inline void prelude() {
    // The memory address of the memory is passed in x0.
    mov(memBase, x0);
    // We want to zero the memory pointer.
    mov(memPtr);
    // Set the up and down counters.
    // mov x11, #1
    writeNext(0xd280002b);
    // mov x12, #-1
    writeNext(0x9280000c);
  }

  // The code that gets executed at the end of the subroutine.
  inline void postlude() {
    // Exit code 0.
    mov(x0);
    // ret
    writeNext(0xd65f03c0u);
  }

  // Move zero to register.
  inline void mov(const Register &dst) {
    // mov x0, #0
    uint32_t instr = 0xd2800000u;
    instr |= dst.encode();
    writeNext(instr);
  }

  // Move register to register.
  inline void mov(const Register &dst, const Register &src) {
    // mov x0, x0
    uint32_t instr = 0xaa0003e0u;
    instr |= dst.encode();
    instr |= (src.encode() << 16);
    writeNext(instr);
  }

  // Move immediate to register.
  inline void mov(const Register &dst, uint16_t imm) {
    // mov x0, #0
    uint32_t instr = 0xd2800000u;
    instr |= dst.encode();
    instr |= (imm << 5);
    writeNext(instr);
  }

  // Load byte from memory, extend rest of register with zero.
  inline void ldrb(const Register &dst,
                   const Register &base,
                   const Register &index) {
    // ldrb w0, [x0, x0]
    uint32_t instr = 0x38606800u;
    instr |= dst.encode();
    instr |= (base.encode() << 5);
    instr |= (index.encode() << 16);
    writeNext(instr);
  }

  // Add two registers and place result into third register.
  inline void add(const Register &dst,
                  const Register &left,
                  const Register &right) {
    // add x0, x0, x0
    uint32_t instr = 0x8b000000u;
    // First register is the destination.
    instr |= dst.encode();
    // Second register is one of the operands.
    instr |= (left.encode() << 5);
    // Third register is the other operand.
    instr |= (right.encode() << 16);
    writeNext(instr);
  }

  // Add with immediate.
  inline void add(const Register &dst, const Register &src, uint16_t imm) {
    assert(imm <= ADD_SUB_IMM_LIMIT); // should fit [0, 4096).
    // add x0, x0, #0
    uint32_t instr = 0x91000000u;
    // First register is the destination.
    instr |= dst.encode();
    // Second register is the operand.
    instr |= (src.encode() << 5);
    // Then the immediate.
    instr |= (imm << 10);
    writeNext(instr);
  }

  // Substract with immediate.
  inline void sub(const Register &dst, const Register &src, uint16_t imm) {
    assert(imm <= ADD_SUB_IMM_LIMIT); // should fit [0, 4096).
    // sub x0, x0, #0
    uint32_t instr = 0xd1000000u;
    // First register is the destination.
    instr |= dst.encode();
    // Second register is the operand.
    instr |= (src.encode() << 5);
    // Then the immediate.
    instr |= (imm << 10);
    writeNext(instr);
  }

  // Add register value to value in memory address.
  inline void ldaddb(const Register &addr, const Register &amt) {
    // ldaddb w0, wzr, [x0]
    uint32_t instr = 0x3820001fu;
    // First register is how much we want to add.
    instr |= (amt.encode() << 16);
    // // Second register is the old value, we ignore.
    // instr |= xzr_sp.encode();
    // Third register is the address.
    instr |= (addr.encode() << 5);
    writeNext(instr);
  }

  // Branch if register is zero.
  // Returns the location, such that the jump can be patched later.
  inline size_t cbz(const Register &reg) {
    // cbz x0, #0
    uint32_t instr = 0x34000000u;
    instr |= reg.encode();
    writeNext(instr);
    size_t where = _instructions.size() - 1;
    return where;
  }

  // Branch if register is not zero.
  // Returns the location, such that the jump can be patched later.
  inline size_t cbnz(const Register &reg) {
    // cbz x0, #0
    uint32_t instr = 0x35000000u;
    instr |= reg.encode();
    writeNext(instr);
    size_t where = _instructions.size() - 1;
    return where;
  }

  // Performs a branch patch given a location and offset (byte aligned).
  inline void patchBranch(size_t index, int32_t indexDifference) {
    uint32_t instr = _instructions[index];
    // AArch64 expects the imm19 to be the address divided by 4.
    // Since we do indices, we do not have to do any processing.
    uint32_t rel = indexDifference;
    // Mask the immediate.
    // This is necessary for negative numbers, not required for positive ones.
    uint32_t toEncode = static_cast<uint32_t>(rel) & ((1 << 19) - 1);
    instr |= (toEncode << 5);
    _instructions[index] = instr;
  }

  // Writes an svc 0x80.
  inline void syscall() {
     writeNext(0xd4001001u);
  }

  // Syscall to print a character out.
  inline void syscallOut() {
    // mov x0, #1
    // adr x1, address (here not relative)
    // mov x2, length
    // mov x16, #4
    // svc 0x80
    mov(x0, 1u);
    add(x1, memBase, memPtr);
    mov(x2, constOne);
    mov(sys, 4u);
    syscall();
  }

  // Syscsall to read a character in.
  inline void syscallIn() {
    // mov x0, #0
    // adr x1, address (here not relative)
    // mov x2, length
    // mov x16, #3
    // svc 0x80
    mov(x0, 0u);
    add(x1, memBase, memPtr);
    mov(x2, constOne);
    mov(sys, 3u);
    syscall();
  }

};

#endif
