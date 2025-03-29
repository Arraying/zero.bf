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


#include <fstream>
#include <iostream>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

constexpr size_t MEMORY_SIZE = 30000;

void getInstructions(char*, std::vector<char>&, std::unordered_map<size_t, size_t>&);

int main(int argc, char** argv) {
  // We expect most of the time the program is provided.
  if (__builtin_expect(argc < 2, false)) {
    std::cerr << "zero: please provide the input file" << std::endl;
    return 1;
  }
  // Keep track of the jumping points;
  std::vector<char> instructions;
  std::unordered_map<size_t, size_t> jump;
  try {
    getInstructions(argv[1], instructions, jump);
  } catch (std::runtime_error &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  // Program metadata.
  size_t instructionLength = instructions.size();
  size_t pc = 0;
  int32_t memory[30000];
  size_t pointer;
  while (pc < instructionLength) {
    switch (instructions[pc]) {
      case '+':
        memory[pointer]++;
        break;
      case '-':
        memory[pointer]--;
        break;
      // Memory pointer: wrap around.
      case '<':
        if (__builtin_expect(pointer == 0, false)) {
          pointer = MEMORY_SIZE - 1;
        } else {
          pointer--;
        }
        break;
      case '>':
        if (__builtin_expect(pointer == MEMORY_SIZE - 1, false)) {
          pointer = 0;
        } else {
          pointer++;
        }
        break;
      // IO: Delegate entirely to C++.
      case '.':
        std::cout << static_cast<char>(memory[pointer]);
        break;
      case ',':
        std::cin >> memory[pointer];
        break;
      // Jumps: continue so we don't increment program counter.
      case '[':
        if (memory[pointer] == 0) {
          pc = jump[pc];
          continue;
        }
        break;
      case ']':
        if (memory[pointer] != 0) {
          pc = jump[pc];
          continue;
        }
        break;
      default:
        std::string errorMessage = "unknown instruction: ";
        errorMessage.push_back(instructions[pc]);
        throw std::runtime_error(errorMessage);
    }
    pc++;
  }
  std::cout << std::endl;
}

// Reads the instructions from file. If it doesn't exist, it does nothing.
// Also populates the jump table.
void getInstructions(char* fileName, std::vector<char> &instructions, std::unordered_map<size_t, size_t> &jump) {
  char ch;
  std::fstream file(fileName, std::fstream::in);
  std::stack<size_t> stack;
  size_t pc = 0;
  while (file >> ch) {
    instructions.push_back(ch);
    switch (ch) {
      case '[':
        stack.push(pc);
        break;
      case ']':
        if (__builtin_expect(stack.size() == 0, false)) {
          throw std::runtime_error("program parse error: expected [");
        }
        size_t begin = stack.top();
        stack.pop();
        jump[begin] = pc;
        jump[pc] = begin;
        break;
    }
    pc++;
  }
  if (!__builtin_expect(stack.empty(), true)) {
    throw std::runtime_error("program parse error: expected ]");
  }
}
