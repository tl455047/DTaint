#include <stdio.h>
#include <unistd.h>

__attribute__((constructor)) void write_cmd_option(int argc, char** argv, char **envp) {
    int k = 65;

    //while(1) {
    printf("csu_init: argc: %d\n", argc);
    for(int i = 0; i < argc; i++) {
        argv[i][0]= (char)k;
        printf("%s\n", argv[i]);
    }
    int *t = ((void *)argv + 0xc - 0xe8);
    printf("&argc: %p, &argv: %p, argv: %p,t: %p\n", &argc, &argv, argv, t);

    *t = 990;
    //int pid = fork();
   // if(pid == 0) {
       // printf("child argc: %d\n", argc);
      //  for(int i = 0; i < argc; i++) {
         //   printf("%s\n", argv[i]);
       // }
       // return;
   // }
    //int status;
    //waitpid(-1, &status, 0);
    
    //k++;
    //if(k > 80)
      //  k = 65;

    //}
}

int main(int argc, char** argv[]) {
    
        printf("&argc: %p, &argv: %p, argv: %p\n", &argc, &argv, argv);
        printf("parent argc: %d\n", argc);
        //for(int i = 0; i < argc; i++) {
        //   printf("%s\n", argv[i]); 
        //}
    
    return 0;
}
