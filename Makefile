.PHONY: all hello mandelbrot clean

CXX = clang++ -Wall -std=c++17

all:
	mkdir -p bin
	$(CXX) -O3 -o bin/zero zero.cpp

dbg:
	mkdir -p bin
	$(CXX) -O0 -g -o bin/zero-debug zero.cpp

jit:
	mkdir -p bin
	$(CXX) -O0 -g -o bin/zero-jit jit.cpp
	codesign -s - -f --entitlements entitlements.plist ./bin/zero-jit
	./bin/zero-jit ./test/helloworld.b

hello: all
	./bin/zero ./test/helloworld.b

mandelbrot: all
	./bin/zero ./test/mandelbrot.b

clean:
	rm -r bin

