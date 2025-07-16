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

#ifndef compiler_hpp
#define compiler_hpp

#include "assembler.hpp"
#include <stack>

class Compiler {
private:
  Assembler* _assembler;
  std::stack<size_t> _jumps;
  int8_t _cellDelta;
  int64_t _pointerDelta;
  char _mem1;
  char _mem2;
  int8_t _skip;
  void _compile(char &c, char &fut1, char &fut2);

public:
  Compiler(Assembler* assembler);

  // Performs a compilation of a single instruction.
  void compile(char &c);

  // Flushes the compilation buffer of instructions.
  void flushCompilationBuffer();

  // Flushes the cell value difference into an instruction.
  void flushCell();

  // Flushes the memory pointer difference into (an) instruction(s).
  void flushPointer();

};
#endif
