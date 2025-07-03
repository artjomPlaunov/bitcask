#ifndef KEYDIR_H_
#define KEYDIR_H_

#include <stdint.h>
#include <stdlib.h>
#include "uthash.h"

typedef struct Metadata {
    uint32_t file_id;
    uint32_t val_len;
    uint64_t offset;
    uint64_t timestamp;
} Metadata;

typedef struct Entry {
    uint8_t *key;       
    size_t key_len;   
    Metadata meta;
    UT_hash_handle hh; 
} Entry;

void keydir_put(uint8_t *key, size_t key_len, Metadata meta);

Metadata *keydir_get(uint8_t *key, size_t key_len);

void keydir_delete(uint8_t *key, size_t key_len);

void keydir_close(void);

void keydir_print();

#endif