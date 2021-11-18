/**
 * This program is used as a test for __memlog_hook4.
 * In this program function memcpy's 2nd, 3rd argument can be
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

    dst = src + (unsigned char)src[0];
    size = (unsigned char)src[1];

    fprintf(stderr, "dst: %p size: %d\n", dst, size);
    
    for (int i = 0; i < MAXSIZE; i++) {
       
        if (i == MAXSIZE - 1) {
            memcpy(dst, src, size);
        }
        
    }

    return 0;

}
