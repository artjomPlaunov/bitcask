#ifndef BITCASK_H
#define BITCASK_H

#include <stdint.h>
#include "kv.h"

typedef struct Bitcask Bitcask;

Bitcask *bc_open(const char *directory, uint32_t max_size);

int bc_put(Bitcask*, Kv*);
int bc_get(Bitcask*, Key*, Value*);
void bc_print_keydir(Bitcask* bc);
void bc_close(Bitcask*);

#endif
