#ifndef _DTAINT_H_
#define _DTAINT_H_
#define TAINTED_MAP_W 2 << 16
#define TAINTED_MAP_H 512
struct __attribute__((__packed__)) tainted_header {
  u32 num_of_update;
  u32 tainted_bytes;
  u32 union_label;
  u32 input_label;
  u32 last_label;
};

struct __attribute__((__packed__)) d_tainted {
  void* pos;
  unsigned int len;
};

struct d_tainted_map {
  //struct d_label_info label_info;
  //struct tainted_header header[TAINTED_MAP_W];
  struct tainted_header header;
  struct d_tainted tainted_data[TAINTED_MAP_W];
  //u32 tainted_bytes;
  //u32 union_label;
};

#endif