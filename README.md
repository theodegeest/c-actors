# C-actors

C-actors is an actor library written in C, where you can define actors in native C. This project is a use case where speedup stacks enable educated guesses about which optimizations might be worthwhile to implement.

# To build

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cd build
ninja
```

# To run

```bash
./main
```

# Run tests

```bash
ninja test
```
