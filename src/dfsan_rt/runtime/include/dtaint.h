#ifndef _DTAINT_H_
#define _DTAINT_H_
#define TAINTED_MAP_W 2 << 14
#define TAINTED_MAP_H 512
struct __attribute__((__packed__)) tainted_header {
  unsigned id;
  unsigned shape;
  unsigned int num;
  unsigned int len;
  void* ptr;
  unsigned long long value;
};

struct __attribute__((__packed__)) d_tainted {
  unsigned int pos;
  unsigned int len;
};

struct d_tainted_map {
  //struct d_label_info label_info;
  //struct tainted_header header[TAINTED_MAP_W];
  //struct d_tainted tainted_data[TAINTED_MAP_W][TAINTED_MAP_H];
  u32 tainted_bytes;
  u32 union_label;
};

#endif