CC = clang
CXX = clang++  
CFLAGS = -O3 -fPIE -pie -g -v 
CXXFLAGS = -O3 -fPIE -pie
OPT=/usr/bin/opt-11
PASS1 = ../build/pass/libDFSanPass.so 
PASS2 = ../build/pass/libTaintPass.so 
PASS1_ARG = -dfsan
PASS2_ARG = -taint
ABILIST1 = ../build/lib/share/dfsan_abilist.txt
ABILIST2 = ../src/pass/abilist.txt
# Need to enable corresponding llvm optimization level, where your pass is registed.
# otherwise your opt pass will not be executed.
LLVM_PASS1 = -Xclang -load -Xclang $(PASS1) -mllvm -taint-dfsan-abilist=$(ABILIST1) 
LLVM_PASS2 = -Xclang -load -Xclang $(PASS2) 
RUNTIME1 = ../build/lib/libclang_rt.dfsan-x86_64.a -ldl -lpthread -lstdc++  
RUNTIME2 = ../build/lib/libtaint_rt.a
SRC := $(wildcard *.c)  
ELF = target
BC = target.bc
BC2 = target2.bc

.PHONY: all origin opt clean

all:
	$(CC) $(CFLAGS) $(LLVM_PASS1) $(LLVM_PASS2) target.c -c -o target.o 
	$(CC) $(CFLAGS) target.o $(RUNTIME1) $(RUNTIME2) -o $(ELF)
	
origin: 
	$(CC) $(CFLAGS) target.c -o target

opt:
	$(CXX) $(CFLAGS) -S -emit-llvm test.cpp -o test.bc 
	$(CXX) $(CFLAGS) -S -emit-llvm -Xclang -load -Xclang ../build/pass/libMemlogPass.so -mllvm -memlog-hook-inst=1 test.cpp -o test2.bc 
#$(OPT) -load ../build/pass/libMemlogPass.so -Memloghook test.bc -o test2.bc
#$(CC) $(CFLAGS) $(BC2) $(RUNTIME2) -o target
	
clean:
	rm -f $(wildcard *.o) 
	rm -f $(ELF)	
	rm -f $(BC)
	rm -f $(BC2)
	rm -f target_taint
		