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

#define __ _assembler->

Compiler::Compiler(Assembler* assembler) : _assembler(assembler) {}

void Compiler::compile(char &c) {
  switch (c) {
    case '+':
      __ add(tmp1, memBase, memPtr);
      __ ldaddb(tmp1, constOne);
      break;
    case '-':
      __ add(tmp1, memBase, memPtr);
      __ ldaddb(tmp1, constNegOne);
      break;
    case '>':
      __ add(memPtr, memPtr, constOne);
      break;
    case '<':
      __ add(memPtr, memPtr, constNegOne);
      break;
    case '[':
      __ ldrb(tmp1, memBase, memPtr);
      _jumps.push(__ cbz(tmp1));
      break;
    case ']': {
      __ ldrb(tmp1, memBase, memPtr);
      // The start and end points are in the program counter.
      uint32_t start = _jumps.top();
      _jumps.pop();
      uint32_t end = __ cbnz(tmp1);
      // However, we need the offsets in actual memory address.
      // This is a bit useless because we will divide by 4 anyway, but it helps
      // in the intermeditate processing.
      // Forward: we jump to the instruction after.
      int32_t deltaF = (static_cast<int32_t>(end) * INS_WIDTH)
                       - (static_cast<int32_t>(start) * INS_WIDTH)
                       + INS_WIDTH;
      __ patchBranch(start, deltaF);
      // Backward: we jump to the instruction after too.
      int32_t deltaB = (static_cast<int32_t>(start) * INS_WIDTH)
                       - (static_cast<int32_t>(end) * INS_WIDTH)
                       + INS_WIDTH;
      __ patchBranch(end, deltaB);
      break;
    }
    case '.':
      __ syscallOut();
      break;
    case ',':
      __ syscallIn();
      break;
  }
}

#undef __
