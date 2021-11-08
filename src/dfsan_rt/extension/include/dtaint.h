#ifndef _DTAINT_H_
#define _DTAINT_H_
#define TAINTED_MAP_W 2 << 16
#define TAINTED_MAP_H 32

struct tainted_header {
  
  unsigned hits : 24;
  // instructions executed
  unsigned id : 24;
  // unique id
  unsigned shape0 : 5;
  unsigned shape1 : 5;
  unsigned shape2 : 5;
  // shape for operands
  // However, instructions for 
  unsigned type : 2;
  // type for instructions
  // memory allocate 
  // reference, dereference
  unsigned attribute : 4;
  // instruction detail ?

  // total offset nodes 
  // total offset bytes 
  unsigned bytes : 24;
  unsigned num : 16;
  unsigned label : 16;
  // Just use for quickly identify offset when executing,
  // no meaning for fuzzer.
  unsigned reserved : 3;

} __attribute__((packed));


struct tainted_operands {

  unsigned long long v0;
  unsigned long long v1;
  unsigned long long v2;
  // dst, src, val
  // How many operands we need to record ?
};

struct d_tainted {
  
  unsigned int pos;
  unsigned int len;

};

struct d_tainted_map {
  
  struct tainted_header headers[TAINTED_MAP_W];
  //struct tainted_operands log[TAINTED_MAP_W][TAINTED_MAP_H]; 
  struct d_tainted offset[TAINTED_MAP_W][TAINTED_MAP_H]; 
  // I think 32 node of offset is sufficient for almost all situation.

};


#endif