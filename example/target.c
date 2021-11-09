#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#define MAXSIZE 8


struct A{
    int pos;
    int len;
};
char dst[MAXSIZE];
int main(int argc, char** argv)
{  
    //char src[MAXSIZE] = {1};
    int src[MAXSIZE][MAXSIZE];
    FILE *f;
	int size, len;
    char buf[16];

    memset(src[0], 0, MAXSIZE);
    /*src = malloc(MAXSIZE);
    
    memset(src, 0, MAXSIZE);

	f = fopen ("test.txt", "r");
	  
    while (1) {
		len = fread (src, 1, 1, f);
		if (len <= 0) 
			break;
	}

    fclose(f);*/

    int temp = 0, count = 0;

    for(int i = 0; i < MAXSIZE; i++) {
        for(int j = 0; j < MAXSIZE; j++) {
            src[i][j] = 9487 + i;
            count += src[i][j];
        }
    }
    
    memcpy(dst, src, MAXSIZE);

    memmove(dst, dst + MAXSIZE/2 , MAXSIZE/4);

    struct A st;
    st.len = 10;
    st.pos = 1;

    count += st.len + st.pos;    
    fprintf(stderr, "%d\n", count);
    //free(src);

    return 0;
}
