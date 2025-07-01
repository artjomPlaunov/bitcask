#include <stdlib.h>
#include <string.h>
#include "kv.h"

struct Kv {
    uint8_t *key;
    int key_size;
    uint8_t *val;
    int val_size;
};

Kv *kv_create(const uint8_t *key, int key_size, const uint8_t *val, int val_size) {
    Kv *kv = malloc(sizeof(Kv));
    if (!kv) return NULL;

    kv->key = malloc(key_size);
    if (!kv->key) {
        free(kv);
        return NULL;
    }
    memcpy(kv->key, key, key_size);

    kv->val = malloc(val_size);
    if(!kv->val) {
        free(kv->key);
        free(kv);
        return NULL;
    }
    memcpy(kv->val, val, val_size);
    return kv;
}
void kv_close(Kv *kv) {
    if (!kv) return;
    free(kv->key);
    free(kv->val);
    free(kv);
}

