#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
int main() {
  int status;
  pid_t pid = fork();

  setenv("GAGAGA", "BADDAASDSADSSS", 1);
  setenv("Angora", "fuzzing", 1);
  
  if (pid == 0) {
    fprintf(stderr, "child exec\n");
    char* argv[1];
    argv[0] = NULL;
    execl("./env_child_test", "rew", NULL); 
  } 
  else {

  }
  
  wait(&status);

  return 0;
}