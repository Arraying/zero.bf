.PHONY: all hello mandelbrot clean

CXX = clang++ -Wall -std=c++17

all: interpreter

interpreter:
	@mkdir -p bin
	@$(CXX) -O3 -o bin/zero-interp interpreter.cpp

jit:
	@mkdir -p bin
	@$(CXX) -O0 -g -o bin/zero-jit jit.cpp assembler.cpp compiler.cpp register.cpp
	@codesign -s - -f --entitlements entitlements.plist ./bin/zero-jit

hello: all
	./bin/zero ./test/helloworld.b

mandelbrot: all
	./bin/zero ./test/mandelbrot.b

clean:
	@rm -r bin

