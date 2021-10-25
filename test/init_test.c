#include <stdio.h>
#include <unistd.h>
#include <string.h>
__attribute__((constructor)) void write_cmd_option(int argc, char** argv, char **envp) {
    char str[64] = "fool";
    //while(1) {
    printf("csu_init: argc: %d\n", argc);
    for(int i = 0; i < argc; i++) {
        strncpy(argv[i], str, strlen(str));
        printf("%s\n", argv[i]);
    }
}

int main(int argc, char** argv[]) {
    
        printf("&argc: %p, &argv: %p, argv: %p\n", &argc, &argv, argv);
        printf("parent argc: %d\n", argc);
        //for(int i = 0; i < argc; i++) {
        //   printf("%s\n", argv[i]); 
        //}
    
    return 0;
}
