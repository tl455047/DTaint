#ifndef _DTAINT_H_
#define _DTAINT_H_

#define TAINTED_MAP_W 65536
#define TAINTED_MAP_H 128
struct __attribute__((__packed__)) tainted_header {
  unsigned id;
  unsigned shape;
  unsigned int num;
  unsigned int len;
  unsigned long long offset;
};

struct __attribute__((__packed__)) d_tainted {
  unsigned int pos;
  unsigned int len;
};

struct d_tainted_map {
  struct tainted_header header[TAINTED_MAP_W];
  struct d_tainted tainted_data[TAINTED_MAP_W][TAINTED_MAP_H];
};

#endif