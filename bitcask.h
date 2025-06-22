#ifndef BITCASK_H
#define BITCASK_H

typedef struct Bitcask Bitcask;

Bitcask *bitcask_open(const char *directory);
void bitcask_close(Bitcask *bc);

#endif
