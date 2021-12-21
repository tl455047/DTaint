#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct tainted {

  unsigned pos;
  unsigned len;
  struct tainted *next;

};

typedef struct tainted* tainted_map[4][4];

int main(int argc, char *argv[]) {

    tainted_map *map;

    if (argc >= 2) {
        map = (tainted_map *)malloc(sizeof(tainted_map));
    
        for(int i = 0; i < 4; i++) {

            memset((*map)[i], 0, sizeof(struct tainted *) * 4);

        }

        for(int i = 0; i < 4; i++) {
            for(int j = 0; j < 4; j++) {
                if ((*map)[i][j] == NULL) {
                    (*map)[i][j] = (struct tainted *)malloc(sizeof(struct tainted));
                    (*map)[i][j]->pos = 9487;
                    (*map)[i][j]->len = 9527;
                }

                fprintf(stderr, "map[%d][%d]: pos: %d len: %d\n", i, j, (*map)[i][j]->pos, (*map)[i][j]->len);
            }fprintf(stderr, "\n");
        }
    }
    
    return 0;
}