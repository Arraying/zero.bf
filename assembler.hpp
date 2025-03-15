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

#include <cstdint>
#include "constants.hpp"

class Assembler {
private:
  uint32_t* _baseAddress;
  uint32_t _pc;
  uint32_t _length;

  void writeNext(uint32_t instr);

public:
  Assembler(uint32_t* baseAddress, uint32_t length);
  ~Assembler();

  inline void ret();

  void prelude(char (&memory)[MEMORY_SIZE]);
  void postlude();
};

#endif
