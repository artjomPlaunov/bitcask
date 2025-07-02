#include <stdlib.h>
#include <string.h>
#include "kv.h"

Kv *
kv_create(const uint8_t *key, uint16_t key_len, const uint8_t *val, uint32_t val_len) {
    Kv *kv = malloc(sizeof(Kv));
    
    kv->key = malloc(sizeof(Key));
    kv->key->key = malloc(key_len);
    kv->key->key_len = key_len;
    memcpy(kv->key->key, key, key_len);

    kv->val = malloc(sizeof(Value));
    kv->val->val = malloc(val_len);
    kv->val->val_len = val_len;
    memcpy(kv->val->val, val, val_len);
    return kv;
}
void kv_close(Kv *kv) {
    if (!kv) return;
    free(kv->key->key);
    free(kv->val->val);
    free(kv->key);
    free(kv->val);
    free(kv);
}

