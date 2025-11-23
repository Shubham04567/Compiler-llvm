# Runtime Bypass of Illegal Memory Accesses

A compiler pass framework built on LLVM that detects illegal memory accesses using AddressSanitizer (ASan) and modifies control flow to skip faulty instructions instead of aborting, enabling graceful error recovery and continued program execution.

**Team Members:**
- Shubham Yadav (112201032)
- Anup Kumar (112201042)

**Project Mentor:** Dr. Unnikrishnan C  
**Institution:** Department of Computer Science, Indian Institute of Technology Palakkad

---

## 🎯 Project Overview

### Problem Statement
Memory-unsafe languages like C and C++ are prone to critical bugs that cause:
- Unpredictable software crashes
- Security vulnerabilities (buffer overflows, use-after-free, code injection)
- Undefined behavior that's difficult to debug and reproduce

### Solution
We've developed **usaclang** - a custom compiler that:
1. Uses ASan's instrumentation to detect memory bugs at runtime
2. **Logs errors instead of aborting** program execution
3. Redirects control flow to skip faulty instructions
4. Enables programs to continue running despite memory safety violations

### Key Features
- **Six LLVM transformation passes** for comprehensive memory safety
- **Control flow graph (CFG) transformation** to bypass illegal operations
- **SSA dominance preservation** through automated CFG repair
- **Runtime logging** of memory violations with source location details
- **~10x compile-time overhead**, **1-5x runtime overhead** (depending on workload)

---

## 🏗️ Architecture

### Core Passes

The framework consists of six LLVM transformation passes that run in sequence:

1. **InitPtrPass** - Initializes all uninitialized pointers to NULL
2. **AsanPass** - Transforms ASan abort blocks into logging and recovery blocks
3. **GEPPass** - Validates base pointers for `getelementptr` instructions
4. **MemcpymvPass** - Checks source/destination pointers for memcpy/memmove
5. **FreePass** - Validates pointers before free() calls (detects double-free, use-after-free)
6. **ReallocPass** - Dynamically resizes memory on out-of-bounds access (experimental)

### Key Mechanisms

#### 1. NextSafeBlock Strategy
Finds the next safe instruction after detecting a fault:
- Uses debug location info to identify instructions from different source lines
- Applies BFS on CFG to find safe basic blocks
- Falls back to SafeExit block to prevent infinite loops

#### 2. Dominance Fix Algorithm
Preserves SSA form after CFG modifications:
- Uses LLVM's Dominator Tree to verify definition-use relationships
- Performs BFS to detect dominance violations
- Redirects violating paths to SafeExit block

#### 3. Runtime Validation Functions
Custom validation functions integrated with ASan APIs:
- `__my_is_valid_base_ptr()` - Validates heap/stack pointers
- `__asan_log_violation()` - Logs errors to `.asanlog` files
- `__sanitizer_get_ownership()` - Checks memory ownership

---

## 📂 Directory Structure

```
.
├── my-llvm-pass/              # ReallocPass (experimental, standalone)
│   ├── lib/
│   │   ├── ReallocPass.cpp
│   │   └── asan_runtime_simple.cpp
│   ├── input/                 # Test files
│   └── Makefile
│
├── outoftree/                 # Main usaclang framework
│   ├── include/               # Pass header files
│   │   ├── AsanPass.h
│   │   ├── InitPtrPass.h
│   │   ├── GEP.h
│   │   ├── Memcpymv.h
│   │   ├── FreePass.h
│   │   ├── PassUtils.h        # Core utility functions
│   │   └── logger.h
│   │
│   ├── src/                   # Pass implementations
│   │   ├── AsanPass.cpp
│   │   ├── InitPtrPass.cpp
│   │   ├── GEP.cpp
│   │   ├── Memcpymv.cpp
│   │   ├── FreePass.cpp
│   │   └── logger.cpp
│   │
│   ├── test/                  # Comprehensive test suite
│   │   ├── miniAsanTest/      # Basic ASan validation
│   │   ├── miniGEPandMemcpyTest/
│   │   ├── PerPassTest/       # Individual pass tests
│   │   ├── FullCoverage/      # Integration tests
│   │   ├── SLinkedList/       # Data structure tests
│   │   ├── bfs/               # Multi-source program test
│   │   ├── multithreading/    # Concurrent execution tests
│   │   └── runall.sh          # Run all tests
│   │
│   ├── benchmarking/          # Performance evaluation
│   │   ├── threading/         # Multi-threaded workload
│   │   ├── bfs/               # Graph traversal
│   │   └── replacement_policies/  # Cache simulation
│   │
│   ├── install.sh             # System-wide installation script
│   └── CMakeLists.txt
│
└── README.md

MyLLVMPass directory was initially used for learning:
    now it is not part of main development
```

---

## 🚀 Installation & Usage

### Prerequisites
- LLVM 12+ (with development headers)
- CMake 3.13+
- Clang compiler
- Linux/Unix environment
- C++ compiler

### Building usaclang

```bash
cd outoftree
mkdir build
cd build
cmake ..
make
```

### System-Wide Installation

```bash
cd outoftree
chmod +x install.sh
./install.sh
```

This installs `usaclang` as a system-wide compiler that automatically applies all five core passes.

### Using usaclang

Once installed, use it like any standard compiler:

```bash
# Compile a single file
usaclang program.c -o program

# Compile with optimizations
usaclang -O2 program.c -o program

# Multi-file compilation
usaclang file1.c file2.c -o program
```

### Output
- **Executable**: Normal executable file
- **Log file**: `<source_name>.asanlog` - Contains logged memory violations with source locations

---

## 🧪 Testing

### Running All Tests

```bash
cd outoftree/test
chmod +x runall.sh
./runall.sh
```

### Test Categories

#### 1. **miniAsanTest** - Basic ASan functionality
- Heap buffer overflow
- Stack buffer overflow  
- Use-after-free

#### 2. **miniGEPandMemcpyTest** - Pointer operations
- NULL pointer dereference
- NULL memcpy source/destination
- Valid baseline tests

#### 3. **PerPassTest** - Individual pass validation
- Each pass tested in isolation
- Integration tests combining multiple passes

#### 4. **FullCoverage** - Comprehensive scenarios
- Complex nested structures
- Multiple error conditions
- Edge cases

#### 5. **SLinkedList** - Data structure stress tests
- NULL head/node handling
- Use-after-free in linked lists
- Double-free detection
- Invalid free operations

#### 6. **bfs** - Multi-source program
- Correct and faulty implementations
- Queue and linked list operations
- Cross-file error handling

#### 7. **multithreading** - Concurrent execution
- Thread-safe error recovery
- Lock handling during recovery
- Race condition scenarios

### Running Individual Test Suites

```bash
cd outoftree/test/<test_directory>
chmod +x run.sh
./run.sh
```

---

## 📊 Benchmarking

### Running Benchmarks

```bash
cd outoftree/benchmarking/<benchmark_name>
chmod +x benchmark.sh
./benchmark.sh
```

Results are written to `time.txt` with comparisons between GCC, Clang, and usaclang.

### Benchmark Results

| Program | Compiler | Compile Time | Runtime | Notes |
|---------|----------|--------------|---------|-------|
| **threading** | gcc | 0.44s | 0.05s | Multi-threaded memory ops |
| | clang | 0.56s | 0.05s | |
| | usaclang | 5.02s | 0.16s | **~10x compile, ~3x runtime** |
| **bfs** | gcc | 0.48s | 3.48s | Graph traversal |
| | clang | 0.45s | 2.55s | |
| | usaclang | 5.45s | 5.24s | **~10x compile, ~2x runtime** |
| **replacement_policies** | gcc | 0.60s | 9.19s | Cache simulation |
| | clang | 0.66s | 9.01s | |
| | usaclang | 7.44s | 42.12s | **~10x compile, ~5x runtime** |

### Performance Characteristics
- **Compile time**: ~10x slower due to BFS-based safe block finding and dominance fixing
- **Runtime overhead**: 1-7x depending on memory operation density
- **Note**: Here GCC/Clang are  excluding ASan (which adds ~2x overhead itself) 

---

## 🧩 ReallocPass (Experimental)

A standalone pass that dynamically resizes memory on out-of-bounds access.

### Features
- Automatic heap reallocation on OOB access
- Base pointer tracking and updating
- Offset calculation for derived pointers
- Runtime recovery without crashes

### Building & Testing

```bash
cd my-llvm-pass
mkdir build
cd build
cmake ..
make
cd ..
make  # Tests file01.c
```

### Current Status
⚠️ **Experimental** - Fails on some critical test cases with complex pointer aliasing. Not yet integrated into usaclang.

---

## 🔍 Technical Details

### How It Works

1. **ASan Instrumentation**: Source code compiled with `-fsanitize=address`
2. **Check Interception**: ASan checks are redirected to custom handlers
3. **Error Detection**: Invalid memory access detected at runtime
4. **Control Flow Modification**: 
   - Split basic block at faulty instruction
   - Insert validation checks
   - Create logging block
   - Redirect to safe continuation point
5. **Logging**: Write error details to `.asanlog` file
6. **Recovery**: Continue execution from next safe instruction

### Supported Error Types

- ✅ Stack buffer overflow
- ✅ Heap buffer overflow
- ✅ Use-after-free
- ✅ Double-free
- ✅ Invalid free (freed pointer at offset)
- ✅ NULL pointer dereference
- ✅ Uninitialized pointer usage
- ✅ Invalid memcpy/memmove operations (without buffer checks)

### Limitations

- **Multithreaded programs**: May cause deadlock if jumping out of critical sections
- **Complex loops**: May exit early to prevent infinite error loops
- **Aliased pointers**: Some complex aliasing patterns not fully handled (ReallocPass)
- **Performance**: Higher overhead for memory-intensive applications

---

## 📚 References

1. [LLVM Official Documentation](https://llvm.org/)
2. [LLVM for Grad Students](https://www.cs.cornell.edu/~asampson/blog/llvm.html)
3. [AddressSanitizer Algorithm](https://blog.trailofbits.com/2024/05/16/understanding-addresssanitizer-better-memory-safety-for-your-code/)
4. [LLVM Pass Tutorial](https://llvm.org/devmtg/2014-04/PDFs/Talks/Passes.pdf)
5. [Android ASan Guide](https://developer.android.com/ndk/guides/asan)
6. [Lifelong Optimization Technical Report](https://llvm.org/pubs/2003-09-30-LifelongOptimizationTR.pdf)

---