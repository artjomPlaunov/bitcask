#include "stdlib.h"
#include "kv.h"

struct Kv {
    const char *key;
    int key_size;
    const char *val;
    int val_size;
};

Kv *kv_create(const char *key, int key_size, const char *val, int val_size) {
    Kv *kv = malloc(sizeof(Kv));
    if (!kv) return NULL;


}
void kv_close(Kv *kv) {
    if (!kv) return;
    free(kv);
}

