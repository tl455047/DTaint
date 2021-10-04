# Dora-Taint
This project is for taint analysis based on llvm DataFlowSanitizer, and used for supporting fuzzing.
## Directories
- src
  - compiler
    - A simple clang wrapper for compiling with DFSan.
  - dfsan_rt
    - DataFlowSanitizer runtime library extracted from llvm-11.
    - Some libc IO wrapper is added in dfsan_io.cpp.
  - pass
    - llvm-pass one is DataFlowSanitizer, TaintPass is for special instruction hook for tainting.
  - taint_rt
    - Runtime library for TaintPass. 
## Feature
  - llvm DataFlowSanitizer.
  - Some libc IO wrapper.
  - Clang wrapper for compiling with DFSan directly.
  - Another TaintPass for hooking some special instructions.
## TODO
  - Modify DFSan runtime library to support tracing input offset.