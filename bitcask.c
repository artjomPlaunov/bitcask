#include <stdio.h>
#include <stdlib.h>
#include "bitcask.h"

struct Bitcask {
    const char *directory;
};

Bitcask *bitcask_open(const char *directory) {
    Bitcask *bc = malloc(sizeof(Bitcask));
    if (!bc) return NULL;

    bc->directory = directory; 
    printf("Bitcask opened at: %s\n", directory);
    return bc;
}

void bitcask_close(Bitcask *bc) {
    if (!bc) return;
    printf("Bitcask closed\n");
    free(bc);
}
