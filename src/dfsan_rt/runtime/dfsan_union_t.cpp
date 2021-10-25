
#include "dfsan.h"
#include "dfsan_union_t.h"
#include "sanitizer_common/sanitizer_atomic.h"
#include "sanitizer_common/sanitizer_common.h"
#include "sanitizer_common/sanitizer_file.h"
#include "sanitizer_common/sanitizer_flags.h"
#include "sanitizer_common/sanitizer_flag_parser.h"
#include "sanitizer_common/sanitizer_libc.h"
#include <cstdio>

using namespace __dfsan;

/**
 * Input offset can be represented as a bitVector, the i bit in bitVector
 * represents the i byte in input, if the i bit is 1, means the i byte in
 * input offset is tainted in this moment.
 * 
 * For example, bitVector 111000111000111 means that input offset 0,1,2,6,7,8,12,13,14
 * are tainted, and each dfsan_label will be mapped to an unique bitVector. 
 * In this way, when some of memory byte is labeled as label j, we can obtain
 * the bitVector binded with label j, then we know which input offset affects 
 * this memory byte. 
 * 
 * The simplest way to record the input offset is using C++ STL bitset, but
 * there are several disadvantages, firstly the size of bitVector is decided
 * by the actual input size. when the input size becomes larger and larger,
 * this can waste lots of memory space.
 * 
 * Therefore, use list will be the proper solution, a bitVector can be represented
 * as a set of continuous bit 1, and bit 0, and we only need to record thr set of 
 * continuous bit 1, then we can recover the bitVector easily.
 * 
 * bitVector 111000111000111 can be transformed as a list pos:0,len:3 -> pos:6,len:3 ->
 * pos:12,len:3, the size of list is decided by the number of set of continuous bit 1, 
 * the worst case will be input size/2, like 1010101010..., the best case is only 1 bit 
 * is 1, and we only need one node to store it. In my observation, set of continuous bit
 * 1 remains small. This way is more efficient.  
 * 
 * Although the list can reserve much memory, another problem is that in DFSan we need
 * to do many union operations, in bitVector, we can union it with bitwise or operations,
 * which is quite efficient. However, in list we need to first combine two lists with two
 * different labels, and then handle the overlapped and continuous region, the time complexity 
 * will be O(m+n), m is the number of nodes in list 1, and n is the number of nodes in list 2.
 * After complete the new list, we still need to make sure the list is unique before insert
 * the list into union table. We want the search to be efficient, linear search is not acceptable,
 * since the table size is at least 2x or 3x larger than the input size n. A efficient search
 * method is needed.
 */

/**
 * Offset list store offsets in different element according to 
 * total tainted bytes, this can reduce the search time when finding the
 * offset is existed in union_t or not.
 * The index represents power of 2. [1, 2, 4, 8, 16, 32, 64 ...] 
 * offset_list[1] -> 2^1 = 2 -> store total tainted bytes in [2]
 * offset_list[2] -> 2^2 = 4 -> store total tainted bytes in [3, 4]
 * offset_list[3] -> 2^3 = 8 -> store total tainted bytes in [5, 6, 7, 8]
 * ...
 */
struct offset_node* __union_t_offset_list[DFSAN_UNION_T_OFFSET_LIST_SIZE + 1];

/**
 * Map union table to shadow memory.
 */
static struct offset_node *union_table() {
  return ((struct dfsan_union_t *) UnionTableAddr())->union_t;
}

static struct tainted *offset_table(dfsan_label label) {
  return ((struct dfsan_union_t *) UnionTableAddr())->offset_t[label];
}

void union_t_output_tainted(struct offset_node* node, int fd) {
  struct tainted* t = node->tainted;
  char buf[256];
  for(u32 i = 0; i < node->num; i++) {
    internal_snprintf(buf, sizeof(buf), "pos: %u, len: %u ", t[i].pos, t[i].len);
    WriteToFile(fd, buf, internal_strlen(buf));
    //fprintf(stderr, "pos: %u, len: %u ", node->pos, node->len);
   
  }WriteToFile(fd, "\n", 1);
  internal_snprintf(buf, sizeof(buf), "total tainted bytes: %u, total continuous regions: %u\n", node->len, node->num);
  WriteToFile(fd, buf, internal_strlen(buf));//fprintf(stderr, "\n");
}

static struct offset_node* union_t_offset_union(dfsan_label l1, dfsan_label l2, dfsan_label new_label) {
  // union two tainted list
  struct tainted *t1, *t2, *temp, *new_tainted;
  struct offset_node* new_node;
  u32 size, size1, size2;

  t1 = offset_table(l1);
  t2 = offset_table(l2);
  // Use table[DFSAN_UNION_T_SIZE] as temp.
  temp = offset_table(DFSAN_UNION_T_SIZE); 

  size1 = union_table()[l1].num;
  size2 = union_table()[l2].num;

  // combine two list
  u32 i = 0, j = 0, k = 0;
  while(1) {
    
    if(i >= size1) {
      
      while(j < size2) {
        temp[k++] = t2[j++];
      } 
      break;
    }
    else if(j >= size2) {
      
      while(i < size1) {
        temp[k++] = t1[i++];
      } 
      break; 
    }

    if(t1[i].pos < t2[j].pos) { 
      temp[k] = t1[i++];
    }
    else {
      temp[k] = t2[j++];
    }

    k++;
    
  }
  // whether it is new label, copy the offset to new label address
  new_tainted = offset_table(new_label); 

  // handle overlapped
  i = 0, j = 0;
  s32 overlapped, taint_end, tail_end, total_len = 0, num_of_node = 1;

  new_tainted[j] = temp[i++];
  while(1) {
    
    if(i >= k)
      break;

    tail_end = temp[i].pos + temp[i].len - 1;
    taint_end = new_tainted[j].pos + new_tainted[j].len - 1;
    overlapped = (temp[i].pos - taint_end) * (tail_end - new_tainted[j].pos);

    if(overlapped < 0) {
       // overlapped
      if(taint_end < tail_end) 
        new_tainted[j].len += tail_end - taint_end;
      
      i++;
    }
    else if(taint_end + 1 == temp[i].pos) {
      // can be connected
      new_tainted[j].len += temp[i].len;

      i++;
    }
    else {
      total_len += temp[i].len;
      num_of_node += 1;
      new_tainted[++j] = temp[i++];
    }
  }
  
 
  total_len += new_tainted[j].len;
  
  new_node = &(union_table()[new_label]);

  new_node->len = total_len;
  new_node->num = num_of_node;
  new_node->tainted = new_tainted;
  new_node->next = NULL;
  //union_t_output_tainted(new_node, 2);
  return new_node;
}

int union_t_offset_is_equal(struct tainted *t1, struct tainted *t2, u32 num) {
  
  for(u32 i = 0; i < num; i++) {
    
    if(t1[i].pos != t2[i].pos || t1[i].len != t2[i].len)
      return 0;
  }

  return 1;
}

void output_offset_list(int fd) {
  struct offset_node * node;
  char buf[256];
  for(u32 i = 1; i < DFSAN_UNION_T_OFFSET_LIST_SIZE; i++) {
    node = __union_t_offset_list[i];
    internal_snprintf(buf, sizeof(buf), "offset_list[%u]: ", i);  
    WriteToFile(fd, buf, internal_strlen(buf));
    while(node != NULL) {
      internal_snprintf(buf, sizeof(buf), "label: %u -> ", DFSAN_UNION_T_SIZE - node->label);  
       WriteToFile(fd, buf, internal_strlen(buf));
   
      node = node->next;
    } WriteToFile(fd, "\n", 1);
   
  }
}

void union_t_offset_list_insert(struct offset_node *node, dfsan_label label) {
  u32 offset_idx, size = node->len;
  //get nearest offset size from len.
  for(offset_idx = 0; size >>=1; offset_idx++);
  
  if(offset_idx < DFSAN_UNION_T_OFFSET_LIST_SIZE + 1) {
    node->label = label;
    node->next = __union_t_offset_list[offset_idx];
    __union_t_offset_list[offset_idx] = node;
  }
  else {
    //Report("FATAL: DataFlowSanitizer: offset list out of bound\n");
    //Die(); 
  }
  //fprintf(stderr, "len: %u, size: %u\n", t->len, offset_idx);

}

int union_t_offset_list_search(struct offset_node* new_node, u32 *new_label) {
  struct offset_node *node;
  u32 offset_idx, size = new_node->len;
  //get nearest offset size from len.
  for(offset_idx = 0; size >>=1; offset_idx++);
 
  if(offset_idx < DFSAN_UNION_T_OFFSET_LIST_SIZE + 1) {
    node = __union_t_offset_list[offset_idx];
  
    while(node != NULL) {
      if(new_node->len == node->len &&
        new_node->num == node->num &&
        union_t_offset_is_equal(new_node->tainted, node->tainted, node->num)) {
        
        *new_label = node->label;
        return 1;
      
      }

      node = node->next;
    }

  }
  else {
    //Report("FATAL: DataFlowSanitizer: offset list out of bound\n");
    //Die(); 
  }
  
  return 0;
}

int union_t_offset_is_exist(dfsan_label_info *label_info, struct offset_node* node, dfsan_label *new_label) {
  // Linear search is bad.
  
  // label 0 is this situation exist ?
  if(node->len == 0) {
    *new_label = 0;
    return 1;
  }
  // If total len is 1, no need to search just index union_t directly.
  else if(node->len == 1) {
    *new_label = node->tainted->pos + 1;
    return 1;
  }
  // total len > 1, search offset list table.
  else {
    return union_t_offset_list_search(node, new_label);
  }
}

dfsan_label dfsan_union_t_insert(dfsan_label_info* label_info, u32 pos) {

  label_info->input_label += 1;
  label_info->last_label += 1;
  union_table()[label_info->input_label].num = 1;
  union_table()[label_info->input_label].len = 1;
  union_table()[label_info->input_label].next = NULL;
  union_table()[label_info->input_label].label = label_info->last_label;
  offset_table(label_info->input_label)->pos = pos;
  offset_table(label_info->input_label)->len = 1;
  return label_info->input_label;
}


dfsan_label dfsan_union_t_union(dfsan_label_info* label_info, dfsan_label l1, dfsan_label l2) {
  struct offset_node* node1, *node2, *union_node;
  dfsan_label new_label = 0;
  
  union_node = union_t_offset_union(l1, l2, label_info->union_label - 1);
  // Check if the offset is already exist.
  if(!union_t_offset_is_exist(label_info, union_node, &new_label)) {
    label_info->union_label -= 1;
    label_info->last_label += 1;
    new_label = label_info->union_label;
    //insert into offset list
    union_t_offset_list_insert(union_node, new_label);
  }

  //fprintf(stderr, "\ndfsan_uniont_t_list_union, label1: %u, label2: %u, last label: %u, union label: %u, new label: %u\n"
  //, l1, l2, label_info->last_label, label_info->union_label, new_label);
  
  return new_label;
}

void dfsan_union_t_dump(dfsan_label_info *label_info, int fd) {
    char buf[512];
    fprintf(stderr, "dump Label list, input size: %u, unions label: %u, num of label: %u, union_t size: %u\n",
    label_info->input_label, DFSAN_UNION_T_SIZE - label_info->union_label, 
    label_info->last_label, DFSAN_UNION_T_SIZE);
    u32 unique_label = 0;
    
    for(u32 i = label_info->union_label; i < DFSAN_UNION_T_SIZE; i++) {
      if(union_table()[i].len > 1) {
        unique_label += 1;
        internal_snprintf(buf, sizeof(buf), "Label: %u ", DFSAN_UNION_T_SIZE - i);
        WriteToFile(fd, buf, internal_strlen(buf));
        union_t_output_tainted(&union_table()[i], fd);
      }
    }

    output_offset_list(fd);
    internal_snprintf(buf, sizeof(buf), "dfsan_uniont_t_list_dump\n\
    total label: %u\n\
    union_t size: %u\n\
    input label: %u\n\
    union label: %u\n\
    unique tainted label with more than 1 byte: %u\n\
    tainted bytes: %u\n\
    ", label_info->last_label, DFSAN_UNION_T_SIZE,  label_info->input_label,
     DFSAN_UNION_T_SIZE - label_info->union_label, unique_label, label_info->tainted_bytes);
    WriteToFile(fd, buf, internal_strlen(buf));
    fprintf(stderr, "dump Label info\n");

    


}

void dfsan_union_t_output_offset(dfsan_label label) {

  union_t_output_tainted(&union_table()[label], 2);
}


void* dfsan_union_t_get_offset(dfsan_label label) {
 
  return &union_table()[label];

}
