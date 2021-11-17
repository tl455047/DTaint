#include <stdio.h>
#include <unistd.h>
#include <string.h>
__attribute__((constructor)) void debug(int argc, char** argv, char **envp) {
    fprintf(stderr, "called before main\n");
}

int main(int argc, char** argv[]) {
    
    printf("this main\n");
    
    return 0;
}
