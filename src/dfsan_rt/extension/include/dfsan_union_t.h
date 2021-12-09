#ifndef _DFSAN_UNION_T_H_
#define _DFSAN_UNION_T_H_
#include "dfsan.h"
/**
 * AFL prefers maximum input size is 1MB. Can we follow this condition ?
 * 1MB / 3 = 333KB may be the prefer input size.
 */

#define DFSAN_UNION_T_OFFSET_LIST_SIZE 16
#define DFSAN_UNION_T_SIZE (2 << DFSAN_UNION_T_OFFSET_LIST_SIZE)  
#define DFSAN_UNION_T_MAXIMUM_NODE_NUM (1 << 8)

/**
 * Structure for maintaining total number of label, number of input_label,
 * union_label.
 * If the total number of label are larger than the limit, program
 * should abort, but for now it just does nothing, the result will be useless.
 */
struct dfsan_label_info {

  dfsan_label last_label;
  dfsan_label union_label;
  dfsan_label input_label; 

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

/**
 * Structure for maitaining basic info for corresponding offset.
 * Also used as node in search list to speeding up searching.
 * 
 */
struct __attribute__((__packed__)) offset_node {
  
  u32 num; // total nodes in offset
  u32 len; // total 1 in offset
  dfsan_label label; // corresponding label
  struct tainted *tainted; // pointer to real offset structure
  struct offset_node* next; // use for construct searching list

};

/**
 * Union table for offset node and offset list.
 * .union_t stores offset node for each label.
 * .offset_t stores real offset list for each offset node.
 */
struct dfsan_union_t {

  struct offset_node union_t[DFSAN_UNION_T_SIZE + 2]; // one more element is used as temp.
  struct tainted offset_t[DFSAN_UNION_T_SIZE + 2][DFSAN_UNION_T_MAXIMUM_NODE_NUM];

};

/**
 * Init label_info.
 */
void dfsan_union_t_init();

/**
 * Insert new label into union table, mostly is 
 * created by input.
 */
dfsan_label dfsan_union_t_insert(dfsan_label_info* label_info, u32 pos);

/**
 * Union two labels, and the input offsets behind them.
 * Then search union table, if the offset already exists, return the
 * label correspond to it, if not insert this offset into table
 * with a new label. 
 */
dfsan_label dfsan_union_t_union(dfsan_label_info* label_info, dfsan_label l1, dfsan_label l2);

/**
 * Dump the union table information, when process is terminated.
 * Debug using.
 */
void dfsan_union_t_dump(dfsan_label_info *label_info, int fd);

/**
 * Return offset node for corresponding label, used for hook functions.
 */
void* dfsan_union_t_get_offset(dfsan_label label);

/**
 * Check the total number of label now, if is out of bound, do nothing.
 * It should be aborted directly, however in some case, we need the process
 * terminated normally, even if the result means nothing. 
 */
int dfsan_check_label(dfsan_label_info* label_info, dfsan_label label);

#endif