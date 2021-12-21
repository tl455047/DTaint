#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {

    int ninks = 4, inknameslen = 0;
    char *inknames = (char *)malloc(0xf);
    strncpy(inknames, "not_kitty.tiff\0", 15);
    const char* cp = inknames;
    char * chunk2, *chunk3;

    chunk2 = (char *)malloc(0x50);
    chunk3 = (char *)malloc(0x90); 
    
    free(chunk2);

    while (ninks > 1) {
        cp = strchr(cp, '\0');
        if (cp) {
            cp++;
            inknameslen += (strlen(cp) + 1);
        }
    }
    ninks--;


    return 0;

}