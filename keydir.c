#include "keydir.h"
#include "uthash.h"
#include <string.h>


static Entry* keydir = NULL;  

void keydir_put(uint8_t *key, size_t key_len, Metadata meta) {
    Entry *entry;

    HASH_FIND(hh, keydir, key, key_len, entry);
    if (entry) {
        entry->meta = meta;
        return;
    }

    entry = malloc(sizeof(Entry));
    if (!entry) return;

    entry->key = malloc(key_len);
    if (!entry->key) {
        free(entry);
        return;
    }

    memcpy(entry->key, key, key_len);
    entry->key_len = key_len;
    entry->meta = meta;

    HASH_ADD_KEYPTR(hh, keydir, entry->key, entry->key_len, entry);
}

Metadata *keydir_get(uint8_t *key, size_t key_len) {
    Entry *entry;
    HASH_FIND(hh, keydir, key, key_len, entry);
    return entry ? &entry->meta : NULL;
}

void keydir_delete(uint8_t *key, size_t key_len) {
    Entry *entry;
    HASH_FIND(hh, keydir, key, key_len, entry);
    if (entry) {
        HASH_DEL(keydir, entry);
        free(entry->key);
        free(entry);
    }
}

void keydir_cleanup(void) {
    Entry *entry, *tmp;
    HASH_ITER(hh, keydir, entry, tmp) {
        HASH_DEL(keydir, entry);
        free(entry->key);
        free(entry);
    }
}
