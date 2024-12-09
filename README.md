# LLVM Global Var Counter Pass

This is an LLVM pass that instruments a module to print its accessed global variables.
It works as follows:

- For every reading and writing access a call to a **user-defined** function `checkRangeGlob` is performed.
- The function (which is user-defined, for ease of use) checks the accesses and performs an action (e.g. write to `stdout`)

- The behavior for checking the access can be implemented arbitrarily, i.e. variable addresses, global variables, etc.

## Building

Use the `Makefile` and `llvm-18` (later ones might work, but not tested)

```bash
make 
```

## Usage

In order to use it (at least with one `.cpp` file) you can:

```bash
clang++ -emit-llvm -c target.cpp -o target.ll 
opt -load-pass-plugin=./GlobalAccessInstrumentation.so --passes="global_access_instrumentation" -o instrumented.bc target.ll
clang++ instrumented.bc -o instrumented_target $(LDFLAGS)
```

The function `checkRangeGlob` must be implemented and exist to work properly. 
Otherwise, the compilation does not succeed.

To adapt existing builds and files, you can add a new pass to the `Makefile`:


```bash
CXXPASS=-fpass-plugin=$(BUILD_DIR)/GlobalAccessInstrumentation.so

# Change the CXXFLAGS to include the pass
CXXFLAGS+= $(CXXPASS)

...
```

