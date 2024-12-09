# LLVM Global Var Counter Pass

This is an LLVM pass that instruments a module to print its accessed global variables.
It works as follows:

- For every reading and writing access a call to a **user-defined** function `checkRangeGlob` is performed.
- The function (which is user-defined, for ease of use) checks the accesses and performs an action (e.g. write to `stdout`)

- The behavior for checking the access can be implemented arbitrarily, i.e. variable addresses, global variables, etc.

## How to build

Use the `Makefile` and `llvm-18` (later ones might work, but not tested)

```bash
make 
```

## How to use

In order to use it (at least with one `.cpp` file) you can:

```bash
clang++ -emit-llvm -c target.cpp -o target.ll 
opt -load-pass-plugin=./global_var_counters.so --passes="global_var_counters" -o instrumented.bc target.ll
clang++ instrumented.bc -o instrumented_target $(LDFLAGS)
```

The function `checkRangeGlob` must be implemented and exist to work properly. 
Otherwise, the compilation does not succeed.

TODO: Provide `-Xclang` option to adapt existing Builds and files.