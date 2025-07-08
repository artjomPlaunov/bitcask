#ifndef SCAN_H
#define SCAN_H

#include <stdint.h>

typedef struct Scan {
    int data_file;
    off_t offset;
    off_t file_size;
} Scan;

Scan *scan_init(const char *path);

int scan_next(Scan * s, Kv **kv, time_t* timestamp);

void scan_close(Scan* s);


#endif 