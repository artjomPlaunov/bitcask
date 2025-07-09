#define _XOPEN_SOURCE 700
#include "include_all.h"


Scan *scan_init(const char* path) {
    int fd = open(path, O_RDONLY, 0644);
    if (fd == -1) {
        perror("scan_init: file open");
        return NULL;
    }
    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("scan_init: fstat failed");
        close(fd);
        return NULL;
    };
    Scan *s = malloc(sizeof(Scan));
    s->data_file = fd;
    s->offset = 0;
    s->file_size = st.st_size;
    return s;
}

// Return -1 on error
// Return 0 on success - next kv pair and timestamp are placed in *kv and 
//                       in *timestamp, respectively. kv is dynamically allocated
//                       and the reference is placed in *kv. If we read all 
//                       the records already, then *kv will hold a NULL pointer.
int scan_next(Scan *s, Kv** kv, time_t* timestamp) {
    
    uint16_t key_len;
    uint32_t val_len;
    uint8_t* key;
    uint8_t* val;

    // First check there is enough space to read a header: 
    // (CRC + timestamp + key size + val size)
    //    4 +     4     +    2     +    4
    if (s->offset + 14 >= s->file_size) {
        printf("File too small to contain header, offset=%lu, file_size=%lu\n", s->offset, s->file_size);
        *kv = NULL;
        return 0;
    }
    // read key size and value size and check if there is enough space in the 
    // file to read the key value pair. 
    // TODO - check read. 
    // TODO - also CRC checks. 
    pread(s->data_file, &key_len, 2, s->offset + 8);
    pread(s->data_file, &val_len, 4, s->offset + 10);
    key_len = ntohs(key_len);
    val_len = ntohl(val_len);

    // read timestamp
    // TODO - check read. 
    pread(s->data_file, timestamp, 4, s->offset + 4);
    *timestamp = ntohl(*timestamp);
    // check if there is enough space to read key/value. 
    if (s->offset + 14 + key_len + val_len > s->file_size) {
        printf("not enough space to read key/val\n");
        *kv = NULL;
        return 0;
    }

    // Otherwise, there is enough space in the file to read the key/value 
    // pairs. 
    // TODO -- are the above checks strictly necessary? Assuming we didn't have 
    // TODO -- these checks, under what scenarios can we cause the code to fail?
    // TODO -- Also assume CRC check (not implemented yet). 

    //  Only thing I can think of currently is a disk corruption: if a bit in the 
    // key or value len is corrupted, it could lead us to thinking a key or value 
    // is longer, and thus exceed the file size. Reading the key or value is done 
    // before the CRC, so we don't know about the corruption yet, thus the check 
    // is important. 

    // If there is a corrupted bit but there is still enough space to read 
    // longer/shorter (depending on corruption) values, then the CRC will detect 
    // the error. 

    key = malloc(key_len);
    val = malloc(val_len);
    // TODO - check read.
    pread(s->data_file, key, key_len, s->offset + 14);
    pread(s->data_file, val, val_len, s->offset + 14 + key_len);
    
    Kv* new_kv = kv_create(key, key_len, val, val_len);
    *kv = new_kv;
    free(key);
    free(val);
    s->offset += (14 + key_len + val_len);
    return 0;
}

void scan_close(Scan* s) {
    if (!s) return;
    close(s->data_file);
    free(s);
}