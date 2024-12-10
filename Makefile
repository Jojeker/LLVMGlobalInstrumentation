# Compiler and flags
LLVM_CONFIG = llvm-config-18
CXX = clang++-18
CXXFLAGS = $(shell $(LLVM_CONFIG) --cxxflags --ldflags --system-libs --libs core passes) \
					 -fPIC -shared

# Pass source and output
PASS_SRC = GlobalAccessInstrumentation.cpp
PASS_SO = GlobalAccessInstrumentation.so

# Target
all: $(PASS_SO)

$(PASS_SO): $(PASS_SRC)
	$(CXX) $(CXXFLAGS) $(PASS_SRC) -o $(PASS_SO) 

clean:
	rm -f $(PASS_SO)

.PHONY: all clean

