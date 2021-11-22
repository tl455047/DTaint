CC = clang
CXX = clang++ 
DTAINT_CC = ../../build/clang-wrapper
AFL_CC = ../../../AFLplusplus/afl-cc 
CFLAGS = -fPIE -pie -g -v 
CXXFLAGS = fPIE -pie
SRC = $(wildcard *.c)  
ELF = $(patsubst %.c, %, $(SRC))
ELF_ORIG = $(patsubst %.c, %_orig, $(SRC))
ELF_AFL = $(patsubst %.c, %_afl, $(SRC))
afl_SRC = $(wildcard *.c) 
.PHONY: all origin afl clean

all: $(ELF)

$(patsubst %.c, %, %.c): %.c
	$(DTAINT_CC) -mllvm -memlog-debug=1 $< -o $@
	$(AFL_CC) $< -o $(patsubst %, %_afl, $@)
#$(CC) $< -o $(patsubst %, %_orig, $@)

clean:
	rm -f $(wildcard *.o) 
	rm -f $(ELF)	
	rm -f $(ELF_ORIG)
	rm -f $(ELF_AFL)
		
