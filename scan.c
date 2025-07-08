#include "include_all.h"

Scan *scan_init(const char* path) {
    int fd = open(path, O_RDONLY, 0644);
    if (fd == -1) {
        perror("scan_init: file open failed.");
        return 0;
    }
    Scan *s = malloc(sizeof(Scan));
    s->data_file = fd;
    s->offset = 0;
    return s;
}

int scan_next(Scan *s) {
    return 0;
}

void scan_close(Scan* s) {
    if (!s) return;
    close(s->data_file);
    free(s);
}