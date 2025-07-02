#include <stdint.h>
#include "kv.h"

#ifndef BITCASK_H
#define BITCASK_H

typedef struct Bitcask Bitcask;

Bitcask *bitcask_open(const char *directory);
int bitcask_put(Bitcask*, Kv*);
void bitcask_close(Bitcask*);

#endif
