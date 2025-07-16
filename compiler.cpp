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

#include "compiler.hpp"
#include "constants.hpp"
#include <algorithm>
#include <iostream>

// Macro magic to make life easier.
#define __ _assembler-> 
#define REPEATER(fn, arg)                                                \
uint64_t iters = abs / ADD_SUB_IMM_LIMIT, rem = abs % ADD_SUB_IMM_LIMIT; \
for (uint64_t i = 0; i < iters; i++) {                                   \
  fn(memPtr, memPtr, ADD_SUB_IMM_LIMIT);                                 \
}                                                                        \
fn(memPtr, memPtr, rem);
#define SKIP(n) _skip = n + 1

// Also have a custom null char.
constexpr char NIL = 0;

// Create a blank compiler.
Compiler::Compiler(Assembler* assembler) 
  : _assembler(assembler), _cellDelta(0u), _pointerDelta(0u),
    _mem1(NIL), _mem2(NIL), _skip(0) {}

// Performs the actual compilation.
void Compiler::_compile(char &c, char &fut1, char &fut2) {
  switch (c) {
    case '+':
      flushPointer();
      _cellDelta++;
      break;
    case '-':
      flushPointer();
      _cellDelta--;
      break;
    case '>':
      flushCell();
      _pointerDelta++;
      break;
    case '<':
      flushCell();
      _pointerDelta--;
      break;
    case '[':
      flushCell();
      flushPointer();
      // Try to optimize [-].
      if (__builtin_expect(fut1 == '-' && fut2 == ']', false)) {
        // Move zero to the address at the current memory address.
        __ mov(tmp1);
        __ strb(tmp1, memBase, memPtr);
        SKIP(2);
      } else {
        __ ldrb(tmp1, memBase, memPtr);
        _jumps.push(__ cbz(tmp1));
      }
      break;
    case ']': {
      flushCell();
      flushPointer();
      __ ldrb(tmp1, memBase, memPtr);
      // The start and end points are in the program counter.
      size_t start = _jumps.top();
      _jumps.pop();
      size_t end = __ cbnz(tmp1);
      // However, we need the offsets in actual memory address.
      // This is a bit useless because we will divide by 4 anyway, but it helps
      // in the intermeditate processing.
      // Forward: we jump to the instruction after.
      int32_t deltaF = static_cast<int32_t>(end)
                       - static_cast<int32_t>(start)
                       + 1;
      __ patchBranch(start, deltaF);
      // Backward: we jump to the instruction after too.
      int32_t deltaB = static_cast<int32_t>(start)
                       - static_cast<int32_t>(end)
                       + 1;
      __ patchBranch(end, deltaB);
      break;
    }
    case '.':
      flushCell();
      flushPointer();
      __ syscallOut();
      break;
    case ',':
      flushCell();
      flushPointer();
      __ syscallIn();
      break;
    default:
      assert(false); // should never get an illegal instruction.
  }

}

// Compile an individual character.
void Compiler::compile(char &current) {
  // Ensure to only compile valid instructions.
  // This makes sure that e.g. whitespace is not in the buffer.
  switch (current) {
    case '+':
    case '-':
    case '>':
    case '<':
    case '[':
    case ']':
    case '.':
    case ',':
    // Allow null which happens at the end of the buffer.
    case NIL:
      _skip = std::max(_skip - 1, 0);
      if (_mem1 != NIL && _skip == 0) {
        Compiler::_compile(_mem1, _mem2, current);
      }
      // Shift everything left by one: mem2 -> mem1, current -> mem2
      _mem1 = _mem2;
      _mem2 = current;
      break;
  }
}

// Flush with phantom characters such that the buffer is emptied.
void Compiler::flushCompilationBuffer() {
  char nil = NIL;
  // Our buffer is of size two so we feed two nil characters.
  Compiler::compile(nil);
  Compiler::compile(nil);
}

void Compiler::flushCell() {
  // Only flush if there is something to flush.
  if (_cellDelta == 0) {
    return;
  }
  // Write the address to tmp1 and the value to write to tmp2.
  __ add(tmp1, memBase, memPtr);
  // This will treat the signed offset as an unsigned value, but that is fine
  // given that the mov instruction with immediate supports signed values.
  __ mov(tmp2, _cellDelta);
  __ ldaddb(tmp1, tmp2);
  _cellDelta = 0;
}

void Compiler::flushPointer() {
  // Only flush if there is something to flush.
  if (_pointerDelta == 0) {
    return;
  }
  uint64_t abs = std::abs(_pointerDelta);
  if (_pointerDelta > 0) {
    REPEATER(__ add, abs);
  } else {
    REPEATER(__ sub, abs);
  }
  _pointerDelta = 0;
}

#undef SKIP
#undef REPEATER
#undef __
