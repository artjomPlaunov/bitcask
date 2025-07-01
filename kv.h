#ifndef KV_H
#define KV_H

#include <stdint.h>

typedef struct Kv Kv ;

Kv *kv_create(const uint8_t *key, int key_size, const uint8_t *val, int val_size);
void kv_close(Kv *kv);

#endif