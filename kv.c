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

uint32_t kv_serialize(Kv *kv, uint8_t **buf, time_t time) {
    uint16_t key_len_be = htons(kv->key->key_len);
    uint32_t val_len_be = htonl(kv->val->val_len);
    int crc = htonl(69);
    uint32_t len = 4 + 4 + 2 + 4 + kv->key->key_len + kv->val->val_len;
    *buf = malloc(len);
    uint8_t *p = (uint8_t *)*buf;
    memcpy(p, &crc, 4);                             p += 4; 
    memcpy(p, &time, 4);                            p += 4;
    memcpy(p, &key_len_be, 2);                      p += 2;
    memcpy(p, &val_len_be, 4);                      p += 4;
    memcpy(p, kv->key->key, kv->key->key_len);      p += kv->key->key_len;
    memcpy(p, kv->val->val, kv->val->val_len); 
    return len;
}

