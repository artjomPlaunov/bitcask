#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../bitcask.h"
#include "../../keydir.h"

#define NUM_KEYS 500

void generate_key_value(size_t i, char *key, size_t klen, char **val, size_t *vlen) {
    snprintf(key, klen, "stress_key_%zu", i);

    size_t len = (i % 3 == 0) ? 16 : (i % 3 == 1) ? 64 : 128;
    *val = malloc(len + 1);
    for (size_t j = 0; j < len; j++) {
        (*val)[j] = 'A' + (j + i) % 26;
    }
    (*val)[len] = '\0';
    *vlen = len;
}

int main(void) {
    const char *dir = "./tests/test_2/data";

    Bitcask *db = bitcask_open(dir);
    if (!db) {
        fprintf(stderr, "Failed to open Bitcask\n");
        return 1;
    }

    // Phase 1: Write lots of keys
    for (size_t i = 0; i < NUM_KEYS; i++) {
        char key[64];
        char *val = NULL;
        size_t vlen;

        generate_key_value(i, key, sizeof(key), &val, &vlen);
        Kv *kv = kv_create((const uint8_t *)key, strlen(key), (const uint8_t *)val, vlen);
        assert(kv != NULL);
        assert(bitcask_put(db, kv) == 0);
        kv_close(kv);
        free(val);
    }

    // Phase 2: Read and verify all keys
    for (size_t i = 0; i < NUM_KEYS; i++) {
        char key[64], *expected_val;
        size_t vlen;
        generate_key_value(i, key, sizeof(key), &expected_val, &vlen);

        Key k = {.key = (uint8_t *)key, .key_len = strlen(key)};
        Value val = {0};
        assert(bitcask_get(db, &k, &val) == 0);
        assert(val.val != NULL);
        assert(val.val_len == vlen);
        assert(memcmp(val.val, expected_val, vlen) == 0);
        free(val.val);
        free(expected_val);
    }

    bitcask_close(db);
    keydir_cleanup();

    printf("Stress test passed: %d keys written and verified.\n", NUM_KEYS);
    return 0;
}
