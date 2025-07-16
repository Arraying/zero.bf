# zero.bf

A project that intends to run a Brainfuck version of Mandelbrot in < 500ms.
Achieved using a JIT compiler.
This project solely targets AArch64 macOS binaries.

## Building and Running

Use `make`, the default target will build an optimized binary.
Run with `./bin/zero-jit path/to/file.b`.
For example, `./bin/zero-jit ./test/mandelbrot.b`.

A 50000-sized `uint8_t` array serves as the memory.
The interpreter will wrap the pointer around.
The JIT compiler does not wrap and will cause a segmenation fault.

# Development

Current attained peak performance: 800 ms.

Debug builds can be generated with `make debug-interpreter` or `make debug-jit`.
To benchmark, `make bench` runs an optimized JIT agains `mandelbrot.b`.
This includes some `make` overhead unfortunately.

List of planned optimizations:
- Optimize file reading (`wc` inspiration).
- Optimize write/read with `printf`/`scanf`.

Other things:
- Safety in JIT mode by checking `[]` bounds.

