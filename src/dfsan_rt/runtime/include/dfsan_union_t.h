#ifndef _DFSAN_UNION_T_H_
#define _DFSAN_UNION_T_H_
#include "../../dfsan/dfsan.h"
/**
 * AFL prefer maximum input size is 1MB. Can we follow this condition ?
 * 1MB / 3 = 333KB may be the prefer input size.
 */

#define DFSAN_UNION_T_OFFSET_LIST_SIZE 20
#define DFSAN_UNION_T_SIZE (1 << DFSAN_UNION_T_OFFSET_LIST_SIZE)  

struct dfsan_label_info {
  dfsan_label last_label;
  dfsan_label union_label;
  dfsan_label input_label;
  void* union_t[DFSAN_UNION_T_SIZE];
};

//struct dfsan_label_info __dfsan_label_info;
void dfsan_union_t_init();
/**
 * Insert new label into union table, mostly is 
 * created by input.
 */
void dfsan_union_t_insert(dfsan_label_info* label_info, u32 pos);
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

#endif