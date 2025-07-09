#include <assert.h>
#include <string.h>
#include <stdint.h>
#include "../../bitcask.h"
#include <stdio.h>
#include "../../keydir.h"

int main(void) {
    Bitcask *db = bc_open("./tests/test_1/data", 100);
    if (!db) {
        fprintf(stderr, "Failed to open Bitcask\n");
        return 1;
    }
    bc_print_files(db);
    bc_close(db);

    printf("All tests passed.\n");
    return 0;
}