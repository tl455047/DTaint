#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#define MAXSIZE 64


char dst[MAXSIZE];
int main(int argc, char** argv)
{  
    //char src[MAXSIZE] = {1};
    int* src;
    FILE *f;
	int size, len;
    char buf[16];

    src = malloc(MAXSIZE);
    
    memset(src, 0, MAXSIZE);

	f = fopen ("test.txt", "r");
	  
    while (1) {
		len = fread (src, 1, 1, f);
		if (len <= 0) 
			break;
	}

    fclose(f);

    int temp = 0, count = 0;

    for(int i = 0; i < MAXSIZE; i++, temp++) {
        src[i] = i;
        
        count += src[i];
    }
    
    for(int i = 0; i < MAXSIZE; i++, temp++) {
        src[i] += 9487;
        fprintf(stderr, "%d\n", src[i]);
    }fprintf(stderr, "\n");
    
    memcpy(dst, src, MAXSIZE);

    memmove(dst, dst + MAXSIZE/2 , MAXSIZE/4);
        
    fprintf(stderr, "%d\n", count);
    free(src);

    return 0;
}
