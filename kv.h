#ifndef KV_H
#define KV_H

typedef struct Kv Kv ;

Kv *kv_create(const char *key, int key_size, const char *val, int val_size);
void kv_close(Kv *kv);

#endif