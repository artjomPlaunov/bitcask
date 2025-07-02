#ifndef KV_H
#define KV_H

#include <stdint.h>

typedef struct Kv {
    uint8_t *key;
    uint16_t key_len;
    uint8_t *val;
    uint32_t val_len;
} Kv;

Kv *kv_create(const uint8_t *key, uint16_t key_len, const uint8_t *val, uint32_t val_len);
void kv_close(Kv *kv);

#endif