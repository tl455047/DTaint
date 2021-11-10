#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#define MAXSIZE 64


struct A{
    int pos;
    int len;
};
char dst[MAXSIZE];
int main(int argc, char** argv)
{  
    char* src;
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

    src = realloc(src, MAXSIZE * 2);
    int temp = 0, count = 0;

    for(int i = 0; i < MAXSIZE; i++) {
       
        src[i] = 9487 + i;
        count += src[i];
    
    }
    
    memcpy(dst, src, MAXSIZE);

    memmove(dst, dst + MAXSIZE/2 , MAXSIZE/4);

    memcmp(dst, src, MAXSIZE);
    struct A st;
    st.len = 10;
    st.pos = 1;

    count += st.len + st.pos;    
    fprintf(stderr, "%d\n", count);
    
    free(src);

    return 0;
}
