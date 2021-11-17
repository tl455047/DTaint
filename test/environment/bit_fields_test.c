#include <stdio.h>
#include <string.h>

/* define simple structure */
struct {
   unsigned int widthValidated;
   unsigned int heightValidated;
} status1;

/* define a structure with bit fields */
struct {
   char widthValidated : 1;
   char heightValidated : 1;
} status2;

struct {
   unsigned int age: 3;
} Age;

struct cmp_header {

  unsigned hits : 24;
  unsigned id : 24;
  unsigned shape : 5;
  unsigned type : 2;
  unsigned attribute : 4;
  unsigned reserved : 5;

} __attribute__((packed)) __cmp_header;

int main( ) {
   unsigned k;
   printf("unsigned size: %d\n", sizeof(k));
   printf( "Memory size occupied by status1 : %d\n", sizeof(status1));
   printf( "Memory size occupied by status2 : %d\n", sizeof(status2));
   printf( "Memory size occupied by Age : %d\n", sizeof(Age));
   printf( "Memory size occupied by __cmp_header: %d\n", sizeof(__cmp_header));
   return 0;
}
