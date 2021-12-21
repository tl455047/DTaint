#include <stdio.h>



int main(int argc, char **argv, char **envp) {
   
   int k = 0;
  while(envp[k] != NULL) {
    fprintf(stderr, "envp[%d]: %s\n", k, envp[k]);
    k++;
  }

  return 0;
}