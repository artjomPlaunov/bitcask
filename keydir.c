#include "keydir.h"
#include <string.h>
#include <stdio.h>

Keydir *keydir_create(void) {
    Keydir *kd = malloc(sizeof(Keydir));
    if (!kd) return NULL;
    kd->entries = NULL;
    return kd;
}

void keydir_print(Keydir *kd) {
    Entry *entry, *tmp;
    printf("======= keydir contents =======\n");

    HASH_ITER(hh, kd->entries, entry, tmp) {
        printf("Key (ascii): ");
        for (size_t i = 0; i < entry->key_len; ++i) {
            char c = entry->key[i];
            putchar((c >= 32 && c <= 126) ? c : '.');
        }

        printf("\nKey (hex)  : ");
        for (size_t i = 0; i < entry->key_len; ++i) {
            printf("%02x ", entry->key[i]);
        }

        printf("\nfile_id    : %u\n", entry->meta.file_id);
        printf("offset     : %lu\n", entry->meta.offset);
        printf("val_len    : %u\n", entry->meta.val_len);
        printf("timestamp  : %lu\n", entry->meta.timestamp);
        printf("--------------------------------\n");
    }

    printf("======= end of keydir ==========\n");
}

void keydir_put(Keydir *kd, uint8_t *key, size_t key_len, Metadata meta) {
    Entry *entry;

    HASH_FIND(hh, kd->entries, key, key_len, entry);
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

    HASH_ADD_KEYPTR(hh, kd->entries, entry->key, entry->key_len, entry);
}

Metadata *keydir_get(Keydir *kd, uint8_t *key, size_t key_len) {
    Entry *entry;
    HASH_FIND(hh, kd->entries, key, key_len, entry);
    return entry ? &entry->meta : NULL;
}

void keydir_delete(Keydir *kd, uint8_t *key, size_t key_len) {
    Entry *entry;
    HASH_FIND(hh, kd->entries, key, key_len, entry);
    if (entry) {
        HASH_DEL(kd->entries, entry);
        free(entry->key);
        free(entry);
    }
}

void keydir_close(Keydir *kd) {
    Entry *entry, *tmp;
    HASH_ITER(hh, kd->entries, entry, tmp) {
        HASH_DEL(kd->entries, entry);
        free(entry->key);
        free(entry);
    }
    free(kd);
}
