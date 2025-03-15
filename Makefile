.PHONY: all hello mandelbrot clean

CXX = clang++ -Wall -std=c++17

all: interpreter

interpreter:
	@mkdir -p bin
	@$(CXX) -O3 -o bin/zero-interp interpreter.cpp

# TODO: Make the binary just zero so the test scripts pick up on it.
jit:
	@mkdir -p bin
	@$(CXX) -O0 -g -o bin/zero-jit jit.cpp assembler.cpp
	@codesign -s - -f --entitlements entitlements.plist ./bin/zero-jit
	./bin/zero-jit ./test/helloworld.b

hello: all
	./bin/zero ./test/helloworld.b

mandelbrot: all
	./bin/zero ./test/mandelbrot.b

clean:
	@rm -r bin

