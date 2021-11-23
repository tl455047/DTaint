/**
 * This program is used as a test for __memlog_hook6.
 * In this program function free argument can be
 * affected by input.
 */

#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define MAXSIZE 512

int main(int argc, char** argv)
{  
    char src[MAXSIZE];
    FILE *f;
	unsigned int size, len;
    char *dst;

    if (argc < 2)
        return -1;

    memset(src, 0, MAXSIZE);

	f = fopen (argv[1], "r");
	
    if (f == NULL)
        return -1;

    while (1) {
		len = fread (src, 1, MAXSIZE, f);
		if (len <= 0) 
			break;
	}
    
    fclose(f);

    size = (unsigned char)src[1] + (unsigned char)src[4];
    dst = malloc(MAXSIZE);

    fprintf(stderr, "dst: %p size: %d\n", dst, size);
   

    for (int i = 0; i < size; i++) {
       
       dst[i] = src[i]*2;
        
    }

    free(dst);
    
    return 0;

}
