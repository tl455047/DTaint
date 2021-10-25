#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
int main(int argc, char* argv[]) {
    
    int fd = shm_open("shm_test_shm1", O_CREAT | O_RDWR | O_EXCL, 0600);
    fprintf(stderr, "fd: %u", fd);
    while(1);
    return 0;
}
