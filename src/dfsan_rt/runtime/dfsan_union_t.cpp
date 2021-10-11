
#include "../dfsan/dfsan.h"
#include "include/dfsan_union_t.h"
#include "../sanitizer_common/sanitizer_atomic.h"
#include "../sanitizer_common/sanitizer_common.h"
#include "../sanitizer_common/sanitizer_file.h"
#include "../sanitizer_common/sanitizer_flags.h"
#include "../sanitizer_common/sanitizer_flag_parser.h"
#include "../sanitizer_common/sanitizer_libc.h"
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
 * Structure for storing a set of continuous bit 1 in offset.
 * For example, an input offset 111000111000111, can be represented
 * as a list of continuous bit 1, list pos:0,len:3 -> pos:6,len:3 ->
 * pos:12,len:3.
 */
struct  __attribute__((__packed__)) tainted {
  u32 pos;
  u32 len;
  struct tainted* next;
};

struct __attribute__((__packed__)) offset_node {
  u32 num;
  u32 len;
  dfsan_label label;
  struct tainted *tainted;
  struct offset_node* next;
};

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


struct tainted* union_t_create_tainted_node(dfsan_label label, u32 len) {
  // creat tainted node
  struct tainted* tainted_node = (struct tainted* )malloc(sizeof(struct tainted));
  
  if(tainted_node != NULL) {
    tainted_node->pos = label;
    tainted_node->len = len;
    tainted_node->next = NULL;
  }
  else {
    Report("FATAL: DataFlowSanitizer: malloc failed\n");
    Die();
  }
  return tainted_node; 
}

struct offset_node* union_t_create_offset_node(struct tainted *t, u32 num, u32 len) {
  
  struct offset_node* node = (struct offset_node* )malloc(sizeof(struct offset_node));
  
  if(node != NULL) {
    node->tainted = t;
    node->num = num;
    node->len = len;
    node->label = 0;
    node->next = NULL;
  }
  else {
    Report("FATAL: DataFlowSanitizer: malloc failed\n");
    Die();
  }
  return node;   
}


void union_t_output_tainted(struct offset_node* node, int fd) {
  struct tainted* t = node->tainted;
  char buf[256];
  while(t != NULL) {
    internal_snprintf(buf, sizeof(buf), "pos: %u, len: %u ", t->pos, t->len);
    WriteToFile(fd, buf, internal_strlen(buf));
    //fprintf(stderr, "pos: %u, len: %u ", node->pos, node->len);
    t = t->next;
  }WriteToFile(fd, "\n", 1);
  internal_snprintf(buf, sizeof(buf), "total tainted bytes: %u, total continuous regions: %u\n", node->len, node->num);
  WriteToFile(fd, buf, internal_strlen(buf));//fprintf(stderr, "\n");
}

static struct offset_node* union_t_offset_union(struct tainted* t1, struct tainted* t2) {
  // union two tainted list
  struct tainted *tail, *new_tainted, *tainted;
  
  new_tainted = union_t_create_tainted_node(0, 1); 
  tail = new_tainted;

  //union_t_output_tainted(t1, 2);
  //union_t_output_tainted(t2, 2);
  
  // combine two list
  while(1) {
    
    if(t1 == NULL) {
      
      while(t2 != NULL) {
        tail->next = union_t_create_tainted_node(t2->pos, t2->len);
        t2 = t2->next;
        tail = tail->next; 
      } 
      break;
    }
    else if(t2 == NULL) {
      
      while(t1 != NULL) {
        tail->next = union_t_create_tainted_node(t1->pos, t1->len);
        t1 = t1->next;
        tail = tail->next; 
      } 
      break; 
    }

    if(t1->pos < t2->pos) { 
      tail-> next = union_t_create_tainted_node(t1->pos, t1->len);
      t1 = t1->next;
    }
    else {
      tail-> next = union_t_create_tainted_node(t2->pos, t2->len);
      t2 = t2->next;
     
    }
    
    tail = tail->next;
    
  }

  tail = new_tainted;
  new_tainted = new_tainted->next;
  free(tail);
  
  tainted = new_tainted;
  tail = tainted->next;
  // handle overlapped

  s32 overlapped, taint_end, tail_end, total_len = 0, num_of_node = 1;
  while(1) {
    
    if(tail == NULL)
      break;

    tail_end = tail->pos + tail->len - 1;
    taint_end = tainted->pos + tainted->len - 1;
    overlapped = (tail->pos - taint_end) * (tail_end - tainted->pos);

    if(overlapped < 0) {
       // overlapped
      if(taint_end < tail_end) 
        tainted->len += tail_end - taint_end;
      
      tainted->next = tail->next;
      free(tail);

      tail = tainted->next;
    }
    else if(taint_end + 1 == tail->pos) {
      // can be connected
      tainted->len += tail->len;

      tainted->next = tail->next;
      free(tail);

      tail = tainted->next;
    }
    else {
      total_len += tainted->len;
      num_of_node += 1;
      tainted = tail;
      tail = tail->next;
    }
  }
  
  /**
   * First node is not real node, it is 
   * used to store total len, and for more
   * efficient search.
   */
  total_len += tainted->len;
  
  struct offset_node * node = union_t_create_offset_node(new_tainted, num_of_node, total_len);
  //union_t_output_tainted(node, 2);
  return node;
}

int union_t_check_label(dfsan_label label) {
  if(label >= DFSAN_UNION_T_SIZE) {
    //Report("FATAL: DataFlowSanitizer: union_t is full\n");
    //Die();
    return 0;
  }
  return 1;
}

int union_t_offset_is_equal(struct tainted *t1, struct tainted *t2) {
  
  while(t1 != NULL && t2 != NULL) {
    
    if(t1->pos != t2->pos || t1->len != t2->len)
      return 0;

    t1 = t1->next;
    t2 = t2->next;
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
      internal_snprintf(buf, sizeof(buf), "label: %u,num: %u, len: %u -> ", DFSAN_UNION_T_SIZE - node->label, node->num, node->len);  
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
    fprintf(stderr, "offset list insert label %u in size: %u\n", DFSAN_UNION_T_SIZE - label, (1 << offset_idx));
  }
  else {
    Report("FATAL: DataFlowSanitizer: offset list out of bound\n");
    Die(); 
  }
  //fprintf(stderr, "len: %u, size: %u\n", t->len, offset_idx);

}

int union_t_offset_list_search(struct offset_node* new_node, u32 *new_label) {
  struct offset_node *node;
  u32 offset_idx, size = new_node->len;
  //get nearest offset size from len.
  for(offset_idx = 0; size >>=1; offset_idx++);
  fprintf(stderr, "offset list search idx: %u, len: %u\n ", offset_idx, new_node->len);

  if(offset_idx < DFSAN_UNION_T_OFFSET_LIST_SIZE + 1) {
    node = __union_t_offset_list[offset_idx];
    output_offset_list(2);
    while(node != NULL) {
      if(new_node->len == node->len &&
        new_node->num == node->num &&
        union_t_offset_is_equal(new_node->tainted, node->tainted)) {
        
        *new_label = node->label;
        return 1;
      
      }

      node = node->next;
    }

  }
  else {
    Report("FATAL: DataFlowSanitizer: offset list out of bound\n");
    Die(); 
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

void dfsan_union_t_insert(dfsan_label_info* label_info, u32 pos) {
  if(union_t_check_label(label_info->last_label)) {
    struct tainted *t = union_t_create_tainted_node(pos, 1);
    label_info->union_t[label_info->input_label] = union_t_create_offset_node(t, 1, 1);
  }
}

void union_t_free_offset(struct offset_node *node) {
  struct tainted *t  = node->tainted, *temp;

  while(t != NULL) {
    temp = t;
    t = t->next;
    free(temp);
  }

  free(node);
}

dfsan_label dfsan_union_t_union(dfsan_label_info* label_info, dfsan_label l1, dfsan_label l2) {
  struct offset_node* node1, *node2, *union_node;
  dfsan_label new_label = 0;

  if(union_t_check_label(label_info->last_label)) {
    node1 = (struct offset_node *)label_info->union_t[l1];
    node2 = (struct offset_node *)label_info->union_t[l2];
    
    union_node = union_t_offset_union(node1->tainted, node2->tainted);
    union_t_output_tainted(union_node, 2);
    // Check if the offset is already exist.
    if(!union_t_offset_is_exist(label_info, union_node, &new_label)) {
      label_info->union_label -= 1;
      label_info->last_label += 1;
      new_label = label_info->union_label;

      if(union_t_check_label(label_info->last_label)) {
        label_info->union_t[new_label] = union_node;
        //insert into offset list
        union_t_offset_list_insert(union_node, new_label);
      }

    }
    else 
      union_t_free_offset(union_node);
  }
  else {
    label_info->last_label += 1;
    new_label = label_info->last_label;
  }
  
  //fprintf(stderr, "\ndfsan_uniont_t_list_union, label1: %u, label2: %u, last label: %u, union label: %u, new label: %u\n"
  //, l1, l2, label_info->last_label, label_info->union_label, new_label);
  
  return new_label;
}

u32 union_t_tainted_byte_count(struct tainted *t) {
  struct tainted* node = t;
  u32 total_count = 0;
  while(node != NULL) {
    total_count += node->len;
    node = node->next;
  }
  return total_count;
}

void dfsan_union_t_dump(dfsan_label_info *label_info, int fd) {
    char buf[512];
    fprintf(stderr, "dump Label list, input size: %u, unions label: %u, num of label: %u, union_t size: %u\n",
    label_info->input_label, DFSAN_UNION_T_SIZE - label_info->union_label, 
    label_info->last_label, DFSAN_UNION_T_SIZE);
    u32 unique_label = 0;
    
    for(u32 i = label_info->union_label; i < DFSAN_UNION_T_SIZE; i++) {
      if(((struct offset_node *)label_info->union_t[i])->len > 1) {
        unique_label += 1;
        internal_snprintf(buf, sizeof(buf), "Label: %u ", DFSAN_UNION_T_SIZE - i);
        WriteToFile(fd, buf, internal_strlen(buf));
        union_t_output_tainted(((struct offset_node *)label_info->union_t[i]), fd);
      }
    }

    output_offset_list(fd);
    internal_snprintf(buf, sizeof(buf), "dfsan_uniont_t_list_dump\n\
    total label: %u\n\
    union_t size: %u\n\
    input label: %u\n\
    union label: %u\n\
    unique tainted label with more than 1 byte: %u\n\
    ", label_info->last_label, DFSAN_UNION_T_SIZE,  label_info->input_label,
     DFSAN_UNION_T_SIZE - label_info->union_label, unique_label);
    WriteToFile(fd, buf, internal_strlen(buf));
    fprintf(stderr, "dump Label info\n");
}



