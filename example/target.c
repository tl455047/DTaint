#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define MAXSIZE 256

char dst[MAXSIZE];
int main(int argc, char** argv)
{  
    char src[MAXSIZE] = {1};
    FILE *f;
	int size;

    fprintf(stderr, "argc: %d\n", argc);

	f = fopen ("test.txt", "r");
	  
    while (1) {
		size = fread (src, 1, sizeof(src), f);
		if (size <= 0) 
			break;
	}

    fclose(f);

    int temp;

    for(int i = 0; i < MAXSIZE; i++) {
        temp = 0;
        temp += src[0];
        if(i > 1)
            temp += src[i - 1];
        if(i < MAXSIZE - 1)
            temp += src[i+1];
        src[i] = temp;
    }
    for(int i = 0; i < MAXSIZE; i++) {
        memset(dst, src[i], 1);
    }
    return 0;
}
