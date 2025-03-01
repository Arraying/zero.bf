.PHONY: all hello mandelbrot clean

CXX = clang++ -Wall -std=c++11

all:
	mkdir -p bin
	$(CXX) -O3 -o bin/zero zero.cpp

dbg:
	mkdir -p bin
	$(CXX) -O0 -g -o bin/zero-debug zero.cpp

hello: all
	./bin/zero ./test/helloworld.b

mandelbrot: all
	./bin/zero ./test/mandelbrot.b

clean:
	rm -r bin

