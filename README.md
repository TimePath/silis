## Build

```shell
cmake -S. -Bcmake-build-debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++
cmake --build cmake-build-debug --parallel 1 --clean-first --verbose
(cd cmake-build-debug; ctest --verbose)  # or --test-dir
```

## Debugger

### GDB

```shell
echo "add-auto-load-safe-path $PWD/.gdbinit" >> ~/.gdbinit
```

### LLDB

```shell
echo "settings set target.load-cwd-lldbinit true" >> ~/.lldbinit
```
