#ifndef KEYDIR_H_
#define KEYDIR_H_

#include <stdint.h>
#include <stdlib.h>
#include "uthash.h"

typedef struct Metadata {
    uint32_t file_id;
    uint32_t val_len;
    uint32_t offset;
    time_t timestamp;
} Metadata;

typedef struct Entry {
    uint8_t *key;       
    size_t key_len;   
    Metadata meta;
    UT_hash_handle hh; 
} Entry;

// opaque handle
typedef struct Keydir {
    Entry *entries;
} Keydir;

Keydir *keydir_create(void);

void keydir_put(Keydir *kd, uint8_t *key, size_t key_len, Metadata meta);

Metadata *keydir_get(Keydir *kd, uint8_t *key, size_t key_len);

void keydir_delete(Keydir *kd, uint8_t *key, size_t key_len);

void keydir_close(Keydir *kd);

void keydir_print(Keydir *kd);

#endif
