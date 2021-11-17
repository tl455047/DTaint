#include <stdio.h>
#include <stdarg.h>

int func(int num1, int num2, int num3, int num_of_idx, ...) {
   
fprintf(stderr, "num1: %d num2: %p num3: %lf num4: %d\n", num1, num2, num3, num_of_idx);

  va_list args;
  va_start(args, num_of_idx);
  
  //for(int j = 0; j < num_of_idx; j++) {
  fprintf(stderr, "float: %lf\n", va_arg(args, double));
  
  fprintf(stderr, "int64: %p\n", va_arg(args, long long));
  long long v1 = va_arg(args, long long);
  long long v2 = va_arg(args, long long);
  fprintf(stderr, "upper: %p lower: %p\n", v1, v2);
  __int128_t val = va_arg(args, __int128);
  fprintf(stderr, "upper: %p lower: %x\n", (long long)(val >> 64),(val));
    //}
  va_end(args);
}
int main() {

    func(99, 98, 87, 5, 777.676567, 666, 0x7fffffffffff8787, 0x7fffffffffff8585, 0x7fffffffffff8686);
    return 0;
}