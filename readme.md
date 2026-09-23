## Overview
Zugswan is a UCI compatible C++ chess engine.

## Building
Zugswan requires:

- A C++20-compatible compiler
- CMake 3.10 or newer

Clone the repository and configure the project:

```bash
git clone https://github.com/MaximR12/Zugswan.git
cd Zugswan
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

Then build the engine:

```bash
cmake --build build --config Release
```