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

#ifndef REGISTER_HPP
#define REGISTER_HPP

#include <cstdint>

// Represents an AArch64 register.
class Register {
private:
  uint8_t _identifier;

public:
  Register(uint8_t identifier);
  uint8_t encode();
};

// Register allocation as follows:
// x9  - the base address of the memory cells.
// x10 - the memory address index.
const Register memBase(9u);
const Register memPtr(10u);

#endif
