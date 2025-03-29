.PHONY: all interpreter jit debug-interpreter debug-jit bench

CXX = clang++ -Wall -std=c++17
INTERPRETER_FILES = interpreter.cpp
JIT_FILES = jit.cpp assembler.cpp compiler.cpp register.cpp

all: jit

interpreter:
	@mkdir -p bin
	@$(CXX) -O3 -o bin/zero-interp $(INTERPRETER_FILES)

jit:
	@mkdir -p bin
	@$(CXX) -O3 -o bin/zero-jit $(JIT_FILES)
	@codesign -s - -f --entitlements entitlements.plist ./bin/zero-jit

debug-interpreter:
	@mkdir -p bin
	@$(CXX) -O0 -g -o bin/zero-interp $(INTERPRETER_FILES)

debug-jit:
	@mkdir -p bin
	@$(CXX) -O0 -g -o bin/zero-jit $(JIT_FILES)
	@codesign -s - -f --entitlements entitlements.plist ./bin/zero-jit

bench: jit
	@time ./bin/zero-jit ./test/mandelbrot.b

clean:
	@rm -r bin

