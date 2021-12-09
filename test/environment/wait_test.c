#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>



int main() {
    fprintf(stderr, "WNOHANG: %u WUNTRACED: %u", WNOHANG, WUNTRACED);
    return 0;
}