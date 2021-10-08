
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

struct tainted* union_t_create_node(dfsan_label label, u32 len) {
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

void union_t_output_tainted(struct tainted* t, int fd) {
  struct tainted* node = t;
  char buf[256];
  u32 total_count = 0, num_of_node = 0;
  while(node != NULL) {
    num_of_node += 1;
    total_count += node->len;
    internal_snprintf(buf, sizeof(buf), "pos: %u, len: %u ", node->pos, node->len);
    WriteToFile(fd, buf, internal_strlen(buf));
    //fprintf(stderr, "pos: %u, len: %u ", node->pos, node->len);
    node = node->next;
  }
  internal_snprintf(buf, sizeof(buf), "total tainted bytes: %u, total continuous regions: %u\n", total_count, num_of_node);
  WriteToFile(fd, buf, internal_strlen(buf));//fprintf(stderr, "\n");
}

static struct tainted* union_t_offset_union(struct tainted* t1, struct tainted* t2) {
  // union two tainted list
  struct tainted *tail, *new_tainted, *tainted;
  
  new_tainted = union_t_create_node(0, 1); 
  tail = new_tainted;

  while(1) {
    
    if(t1 == NULL) {
      
      while(t2 != NULL) {
        tail->next = union_t_create_node(t2->pos, t2->len);
        t2 = t2->next;
        tail = tail->next; 
      } 
      break;
    }
    else if(t2 == NULL) {
      
      while(t1 != NULL) {
        tail->next = union_t_create_node(t1->pos, t1->len);
        t1 = t1->next;
        tail = tail->next; 
      } 
      break; 
    }

    if(t1->pos < t2->pos) { 
      tail-> next = union_t_create_node(t1->pos, t1->len);
      t1 = t1->next;
    }
    else {
      tail-> next = union_t_create_node(t2->pos, t2->len);
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

  s32 overlapped, taint_end, tail_end;
  while(1) {
    
    if(tail == NULL)
      break;

    taint_end = tainted->pos + tainted->len - 1;
    tail_end = tail->pos + tail->len - 1;
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
      tainted = tail;
      tail = tail->next;
    }
  }
  
  return new_tainted;
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

int union_t_offset_is_exist(void** union_t, struct tainted* t, dfsan_label last_label, dfsan_label *new_label) {
  // Linear search is bad.
  u32 size = last_label <= DFSAN_UNION_T_SIZE ? last_label : DFSAN_UNION_T_SIZE;
  for(u32 i = 1; i < size; i++) {
    if(union_t_offset_is_equal(t, (struct tainted *)union_t[i])) {
      *new_label = i;
      return 1;
    }
  }

  return 0;
}

void dfsan_union_t_insert(void** union_t, dfsan_label label, u32 pos) {
  if(union_t_check_label(label))
    union_t[label] = union_t_create_node(pos, 1);
}

dfsan_label dfsan_union_t_union(void** union_t, dfsan_label* last_label, dfsan_label l1, dfsan_label l2) {
  struct tainted* tainted1, *tainted2, *union_tainted;
  dfsan_label new_label = 0;

  if(union_t_check_label(*last_label)) {
  tainted1 = (struct tainted *)union_t[l1];
  tainted2 = (struct tainted *)union_t[l2];
  
  union_tainted = union_t_offset_union(tainted1, tainted2);
  // Check if the offset is already exist.
  if(!union_t_offset_is_exist(union_t, union_tainted, *last_label, &new_label)) {
    *last_label += 1;
    new_label = *last_label;
    
    union_t[new_label] = union_tainted;
  }
  }
  else {
    *last_label += 1;
    new_label = *last_label;
  }
  //fprintf(stderr, "dfsan_uniont_t_list_union, label1: %u, label2: %u, last label: %u, new label: %u\n", l1, l2, *last_label, new_label);
  
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

void dfsan_union_t_dump(void** union_t, dfsan_label last_label, dfsan_label input_label, int fd) {
    char buf[512];
    fprintf(stderr, "dump Label list, input size: %u, num of label: %u, union_t size: %u\n", input_label, last_label, DFSAN_UNION_T_SIZE);
    u32 unique_label = 0;
    u32 size = last_label <= DFSAN_UNION_T_SIZE ? last_label : DFSAN_UNION_T_SIZE;
    for(u32 i = 0; i < size; i++) {
      if(union_t[i] != NULL &&
        union_t_tainted_byte_count((struct tainted* )union_t[i]) > 1) {
        unique_label += 1;
        internal_snprintf(buf, sizeof(buf), "Label: %u ", i);
        WriteToFile(fd, buf, internal_strlen(buf));
        union_t_output_tainted((struct tainted* )union_t[i], fd);
      }
    }
   
    internal_snprintf(buf, sizeof(buf), "dfsan_uniont_t_list_dump\n\
    union_t size: %u\n\
    unique tainted label with more than 1 byte: %u\n\
    ", last_label, unique_label);
    WriteToFile(fd, buf, internal_strlen(buf));
    fprintf(stderr, "dump Label info\n");
}



