#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct A {
    int B[16];
};


int main () {

    struct A *a = (struct A*)malloc(sizeof(struct A));

    memset(a, 0, sizeof(struct A));

    a->B[10] = 2;

    for(int i = 0; i < 16; i++) {

        fprintf(stderr, "A.B[%d]: %d ", i, a->B[i]);
    }fprintf(stderr, "\n");

    a->B[10]++;
    
    for(int i = 0; i < 16; i++) {

        fprintf(stderr, "A.B[%d]: %d ", i, a->B[i]);
    }fprintf(stderr, "\n");

    a->B[0]++;

    for(int i = 0; i < 16; i++) {

        fprintf(stderr, "A.B[%d]: %d ", i, a->B[i]);
    }fprintf(stderr, "\n");


    return 0;
}