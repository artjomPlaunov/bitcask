#include <stdlib.h>
#include <string.h>
#include "kv.h"


Kv *kv_create(const uint8_t *key, uint16_t key_len, const uint8_t *val, uint32_t val_len) {
    Kv *kv = malloc(sizeof(Kv));
    if (!kv) return NULL;
    kv->key_len = key_len;
    kv->val_len = val_len;
    kv->key = malloc(key_len);
    if (!kv->key) {
        free(kv);
        return NULL;
    }
    memcpy(kv->key, key, key_len);

    kv->val = malloc(val_len);
    if(!kv->val) {
        free(kv->key);
        free(kv);
        return NULL;
    }
    memcpy(kv->val, val, val_len);
    return kv;
}
void kv_close(Kv *kv) {
    if (!kv) return;
    free(kv->key);
    free(kv->val);
    free(kv);
}

