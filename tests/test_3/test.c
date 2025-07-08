#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../bitcask.h"
#include "../../keydir.h"

int main(void) {
    const char *dir = "./tests/test_3/data";

    Bitcask *db = bc_open(dir, 100);
    if (!db) {
        fprintf(stderr, "Failed to open Bitcask\n");
        return 1;
    }

    // Initial key-value pairs
    const char *keys[] = {
        "k0", "k1", "k2", "k3", "k4", "k5", "k6", "k7", "k8", "k9"
    };
    const char *values[] = {
        "val0", "val1", "val2", "val3", "val4", "val5", "val6", "val7", "val8", "val9"
    };

    size_t count = sizeof(keys) / sizeof(keys[0]);

    printf("=== Inserting initial key-value pairs ===\n");
    for (size_t i = 0; i < count; i++) {
        Kv *kv = kv_create((const uint8_t *)keys[i], strlen(keys[i]),
                           (const uint8_t *)values[i], strlen(values[i]));
        assert(kv != NULL);
        assert(bc_put(db, kv) == 0);
        kv_close(kv);
        printf("Inserted: key='%s' -> value='%s'\n", keys[i], values[i]);
    }

    // Overwrite a few keys
    printf("\n=== Overwriting some keys ===\n");
    const char *overwrite_keys[] = {"k2", "k4", "k6"};
    const char *new_values[] = {"new_val2", "new_val4", "new_val6"};

    for (size_t i = 0; i < 3; i++) {
        Kv *kv = kv_create((const uint8_t *)overwrite_keys[i], strlen(overwrite_keys[i]),
                           (const uint8_t *)new_values[i], strlen(new_values[i]));
        assert(kv != NULL);
        assert(bc_put(db, kv) == 0);
        kv_close(kv);
        printf("Overwrote: key='%s' -> new value='%s'\n", overwrite_keys[i], new_values[i]);
    }

    // Final readback and print
    printf("\n=== Reading back all keys ===\n");
    for (size_t i = 0; i < count; i++) {
        Key key = {.key = (uint8_t *)keys[i], .key_len = strlen(keys[i])};
        Value val = {0};
        assert(bc_get(db, &key, &val) == 0);
        printf("Read: key='%s' -> value='%.*s'\n", keys[i], (int)val.val_len, val.val);
        free(val.val);
    }

    bc_close(db);

    printf("\nSimple test completed successfully.\n");
    return 0;
}
