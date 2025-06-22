#include <stdio.h>
#include "bitcask.h"
#include "kv.h"

int main(void) {
    Bitcask *db = bitcask_open("./data");
    if (!db) {
        fprintf(stderr, "Failed to open Bitcask\n");
        return 1;
    }

    Kv *kv = kv_create("hello", 5, "world", 5);
    kv_close(kv);
    bitcask_close(db);
    return 0;
}
