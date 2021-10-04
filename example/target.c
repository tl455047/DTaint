#include <string.h>
#include <stdio.h>

#define MAXSIZE 256
int main()
{  
    char src[MAXSIZE] = {1};
    char dst[MAXSIZE] = {0};
    FILE *f;
	int size1, size2;


	f = fopen ("test1.txt", "rw");
	  
    while (1) {
		    size1 = fread (src, 1, sizeof(src), f);
		    if (size1 <= 0) 
			      break;
	  }

    fclose(f);

    f = fopen ("test2.txt", "rw");
	  
    while (1) {
		    size2 = fread (dst, 1, sizeof(dst), f);
		    if (size2 <= 0) 
			      break;
	  }

    //fclose(f);

    memcpy(dst, src, size1);

    int temp;
    temp += dst[0];
    temp += dst[MAXSIZE/2 - 1];
    temp += dst[MAXSIZE/2];
    
    printf("temp: %d\n", temp);
    //fwrite(dst, 1, sizeof(dst), f);

   
    return 0;
}
