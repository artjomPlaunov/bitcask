#include "keydir.h"
#include "uthash.h"
#include <string.h>


static Entry* keydir = NULL;  

#include <stdio.h>  // add at top if not already there

void keydir_print(void) {
    Entry *entry, *tmp;
    printf("======= keydir contents =======\n");

    HASH_ITER(hh, keydir, entry, tmp) {
        printf("Key (ascii): ");
        for (size_t i = 0; i < entry->key_len; ++i) {
            char c = entry->key[i];
            putchar((c >= 32 && c <= 126) ? c : '.');  // print readable chars
        }

        printf("\nKey (hex)  : ");
        for (size_t i = 0; i < entry->key_len; ++i) {
            printf("%02x ", entry->key[i]);
        }

        printf("\nfile_id    : %u\n", entry->meta.file_id);
        printf("offset     : %lu\n", entry->meta.offset);
        printf("value_sz   : %u\n", entry->meta.value_sz);
        printf("timestamp  : %lu\n", entry->meta.timestamp);
        printf("--------------------------------\n");
    }

    printf("======= end of keydir ==========\n");
}


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
