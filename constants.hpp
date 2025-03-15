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

#ifndef constants_hpp
#define constants_hpp

#include <cstddef>

// The number of bytes an instruction is wide. AArch64 is 32-bit.
#define INS_WIDTH 4u

// How many instructions we want as a buffer for prelude/postlude.
#define INS_BUFFER 16u

// We work with a 50k memory cell, could be more, could be less.
// Wrap-around is forbidden, so it's good to have a large space.
constexpr size_t MEMORY_SIZE = 50000;

#endif
