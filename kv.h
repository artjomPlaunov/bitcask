#ifndef KV_H
#define KV_H

#include <stdint.h>
#include <time.h>
#include <arpa/inet.h>
#include <stdlib.h>

typedef struct Key {
    uint8_t *key;
    uint16_t key_len;
} Key;

typedef struct Value {
    uint8_t *val;
    uint32_t val_len;
} Value;

typedef struct Kv {
    Key *key;
    Value *val;
} Kv;

Kv *kv_create
(const uint8_t *key, uint16_t key_len, const uint8_t *val, uint32_t val_len);

void kv_print(Kv *kv);

void kv_close(Kv *kv);

uint32_t kv_serialize(Kv *kv, uint8_t **buf, time_t time);

#endif