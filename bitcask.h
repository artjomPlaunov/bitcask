#include <stdint.h>
#include "kv.h"

#ifndef BITCASK_H
#define BITCASK_H

typedef struct Bitcask Bitcask;

Bitcask *bitcask_open(const char *directory);

int bitcask_put(Bitcask*, Kv*);
int bitcask_get(Bitcask*, Key*, Value*);
void bitcask_close(Bitcask*);

#endif
