#ifndef _DFSAN_UNION_T_H_
#define _DFSAN_UNION_T_H_
#include "dfsan.h"
/**
 * AFL prefer maximum input size is 1MB. Can we follow this condition ?
 * 1MB / 3 = 333KB may be the prefer input size.
 */

#define DFSAN_UNION_T_OFFSET_LIST_SIZE 16
#define DFSAN_UNION_T_SIZE (1 << DFSAN_UNION_T_OFFSET_LIST_SIZE)  
#define DFSAN_UNION_T_MAXIMUM_NODE_NUM (1 << 12)
struct dfsan_label_info {
  dfsan_label last_label;
  dfsan_label union_label;
  dfsan_label input_label; 
  u32 tainted_bytes;
};

/**
 * Structure for storing a set of continuous bit 1 in offset.
 * For example, an input offset 111000111000111, can be represented
 * as a list of continuous bit 1, list pos:0,len:3 -> pos:6,len:3 ->
 * pos:12,len:3.
 */
struct  __attribute__((__packed__)) tainted {
  u32 pos;
  u32 len;
  //struct tainted* next;
};

struct __attribute__((__packed__)) offset_node {
  u32 num;
  u32 len;
  dfsan_label label;
  struct tainted *tainted;
  struct offset_node* next;
};

struct dfsan_union_t {
  struct offset_node union_t[DFSAN_UNION_T_SIZE + 2]; // one more element is used as temp.
  struct tainted offset_t[DFSAN_UNION_T_SIZE + 2][DFSAN_UNION_T_MAXIMUM_NODE_NUM];
};

//struct dfsan_label_info __dfsan_label_info;
void dfsan_union_t_init();
/**
 * Insert new label into union table, mostly is 
 * created by input.
 */
dfsan_label dfsan_union_t_insert(dfsan_label_info* label_info, u32 pos);
/**
 * Union two labels, and the input offsets behind them,
 * search union table, if the offset already exist, return the
 * label correspond to it, if not insert this offset into table
 * with a new label. 
 */
dfsan_label dfsan_union_t_union(dfsan_label_info* label_info, dfsan_label l1, dfsan_label l2);
/**
 * Dump the union table information, when process is terminated.
 * Debug using.
 */
void dfsan_union_t_dump(dfsan_label_info *label_info, int fd);

void dfsan_union_t_output_offset(dfsan_label label);

void* dfsan_union_t_get_offset(dfsan_label label);

int dfsan_check_label(dfsan_label_info* label_info, dfsan_label label);

static struct offset_node* union_t_offset_union(dfsan_label l1, dfsan_label l2, dfsan_label new_label);
#endif