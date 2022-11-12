### Introduction

A simple UCI chess engine implemented using bitboards.

### Installation

Building from source requires a C11-capable compiler and pthreads.
For now, the engine has only been tested on Linux.
It won't work on Windows without some workaround for pthreads.

Compiling the program is done using CMake:
```
mkdir build
cd build
cmake <OPTIONS> ..
cmake --build .
```

To play against the engine, use a UCI capable GUI such as [Arena Chess GUI](www.playwitharena.de)

