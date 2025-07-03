#include <assert.h>
#include <string.h>
#include <stdint.h>
#include "../../bitcask.h"
#include <stdio.h>
#include "../../keydir.h"

int main(void) {
    Bitcask *db = bitcask_open("./tests/test_1/data");
    if (!db) {
        fprintf(stderr, "Failed to open Bitcask\n");
        return 1;
    }

    const char *keys[] = {"key1", "key2", "key3"};
    const char *values[] = {"val1", "val2", "val3"};
    size_t count = sizeof(keys) / sizeof(keys[0]);

    for (size_t i = 0; i < count; i++) {
        Kv *kv = kv_create((const uint8_t *)keys[i], strlen(keys[i]),
                           (const uint8_t *)values[i], strlen(values[i]));
        assert(kv != NULL);
        assert(bitcask_put(db, kv) == 0);
        kv_close(kv);
    }

    for (size_t i = 0; i < count; i++) {
        Key key = {.key = (uint8_t *)keys[i], .key_len = strlen(keys[i])};
        Value val = {0};
        assert(bitcask_get(db, &key, &val) == 0);
        assert(val.val != NULL);
        assert(val.val_len == strlen(values[i]));
        assert(memcmp(val.val, values[i], val.val_len) == 0);
        printf("Got key='%s' value='%.*s'\n", keys[i], (int)val.val_len, val.val);
        free(val.val);
    }

    bitcask_close(db);
    keydir_cleanup();

    printf("All tests passed.\n");
    return 0;
}