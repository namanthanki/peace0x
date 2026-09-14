# peace0x

A chess engine written in C.

I wrote this engine back in 2021 while learning chess programming through Bluefever Software's VICE series on YouTube. It was my first chess engine. The project has since been cleaned up and organized into modern C, keeping the original evaluation and search logic intact.

## Building

Requires a C compiler (`gcc` or `clang`) supporting C17.

### Using Make

```bash
make
```

To build and run the perft test suite:
```bash
make test
```

### Using CMake

```bash
cmake -B build
cmake --build build
```

## Usage

Start the engine in UCI mode:
```bash
./peace0x
```

Run the perft test suite:
```bash
./peace0x --perft
```

## Author

Naman Thanki
