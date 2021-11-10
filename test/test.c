#include <stdio.h>



int main() {
    unsigned hits = 0;
    unsigned current_hits = 1;

    hits = current_hits++;

    fprintf(stderr, "hits: %u current hits: %u\n", hits, current_hits);

    return 0;
}