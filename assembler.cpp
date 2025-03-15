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
#include <pthread.h>
#include "assembler.hpp"

#define __ this->

Assembler::Assembler(uint32_t* baseAddress, uint32_t length)
    : _baseAddress(baseAddress), _pc(0), _length(length) {
  // At this point, we allow JIT writing.
  pthread_jit_write_protect_np(0);
}

Assembler::~Assembler() {
  // Disallow JIT writing again.
  pthread_jit_write_protect_np(1);
  // Clear instruction cache
  char* charAddress = reinterpret_cast<char*>(_baseAddress);
  __builtin___clear_cache(charAddress, charAddress + _length);
}

void Assembler::writeNext(uint32_t instr) {
  assert(_pc < _length);
  _baseAddress[_pc] = instr;
  _pc++;
}

void Assembler::prelude(char (&memory)[MEMORY_SIZE]) {
  // TODO: Implement.
}

void Assembler::postlude() {
  __ ret();
}

void Assembler::ret() {
  __ writeNext(0xd65f03c0);
}

#undef __
