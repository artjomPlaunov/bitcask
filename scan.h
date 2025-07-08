#ifndef SCAN_H
#define SCAN_H

#include <stdint.h>

typedef struct Scan {
    int data_file;
    uint64_t offset;
} Scan;

Scan *scan_init(const char *path);

int scan_next(Scan * s);

void scan_close(Scan* s);


#endif 