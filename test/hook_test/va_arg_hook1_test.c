/**
 * This program is used as a test for __memlog_va_arg_hook1.
 * In this program aggregate type access should be
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

    size = (unsigned char)src[2] + (unsigned char)src[8];
    
    fprintf(stderr, "size: %d\n", size);
   

    for (int i = 0; i < MAXSIZE; i++) {
       
       if(i >= MAXSIZE - 2) {

           src[size + i] = '8';
       }
        
    }

    return 0;

}


