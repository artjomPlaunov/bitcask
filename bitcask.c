#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "bitcask.h"
#include "limits.h"
#include <arpa/inet.h>
#include <time.h>
#include "keydir.h"

#define FNAME_PREFIX ".data"
#define ACTIVE ".active"
#define MAX_SIZE 100

struct Bitcask {
    char *directory;
    char *active_path;
    int max_id;
    int active_file;
};

int get_next_file_id(const char *directory) {
    DIR *dir = opendir(directory);
    if (!dir) {
        perror("opendir failed");
        return -1;
    }
    struct dirent *entry;
    int max_id = -1;
    while ((entry = readdir(dir)) != NULL) {
        const char *name = entry->d_name;
        if (strncmp(name, FNAME_PREFIX, strlen(FNAME_PREFIX)) == 0) {
            const char *start = name + strlen(FNAME_PREFIX);
            char *end;
            long id = strtol(start, &end, 10);
            if (*end == '\0' && id >= 0 && id <= INT_MAX) {
                if ((int)id > max_id) {
                    max_id = (int)id;
                }
            }
        }
    }

    closedir(dir);
    return max_id + 1;
}

char* path_append(char *path, char * suffix) {
    size_t path_len = strlen(path) + 1 + strlen(suffix) + 1;
    char* new_path = malloc(path_len);
    if (!path) {
        fprintf(stderr, "path_append: failed");
        return NULL;
    }
    snprintf(new_path, path_len, "%s/%s", path, suffix);
    return new_path;
}

Bitcask *bitcask_open(const char *directory) {
    
    // note -- should I implement some max directory length check here,
    // since user may pass in a non-null terminated string? 
    struct stat sb;
    // Check if path exists 
    if (stat(directory, &sb) != 0) {
        fprintf(stderr, "bitcask_open: cannot stat '%s': %s\n", 
                        directory, strerror(errno));
        return NULL;
    }
    // Make sure path is a directory. 
    if (!S_ISDIR(sb.st_mode)) {
        fprintf(stderr, "Error: '%s' is not a directory.\n", directory);
        return NULL;
    }
    // Check read and write permissions.
    if (access(directory, R_OK | W_OK) != 0) {
        fprintf(stderr, "Error: no read/write acess to '%s': %s\n",
                        directory, strerror(errno));
        return NULL;
    }

    Bitcask *bc = malloc(sizeof(Bitcask));
    if (!bc) return NULL;

    bc->directory = strdup(directory);
    if(!bc->directory) {
        free(bc);
        return NULL;
    } 
    
    char* active_path = path_append(bc->directory, ACTIVE);
    if (!active_path) {
        fprintf(stderr, "Failed to allocate memory for file path");
        bitcask_close(bc);
        return NULL;
    }
    printf("%s\n", active_path);
    int fd = open(active_path, O_RDWR | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        printf("here\n");
        bitcask_close(bc);
        return NULL;
    }
    bc->active_file = fd;
    bc->active_path = active_path;
    bc->max_id = get_next_file_id(bc->directory);
    printf("%s\n", active_path);

    printf("Bitcask opened at: %s\n", directory);
    printf("next file id: %d\n", get_next_file_id(directory));
    return bc;
}



// -1 return means error
// 0 return means no error, and val is null if not found.
int bitcask_get(Bitcask *bc, Key *key, Value *val) {
    Metadata *m = keydir_get(key->key, key->key_len);
    uint64_t offset = 4 + 4 + 2 + 4 + key->key_len;
    lseek(bc->active_file, (m->offset) + offset, SEEK_SET);
    val->val_len = m->value_sz;
    val->val = malloc(val->val_len);
    read(bc->active_file, val->val, val->val_len);
    return 0;
}

int bitcask_put
(Bitcask *bc, Kv* kv) {
    uint16_t key_len_be = htons(kv->key->key_len);
    uint32_t val_len_be = htonl(kv->val->val_len);
    time_t now = htonl(time(NULL));
    uint64_t offset = lseek(bc->active_file, 0, SEEK_CUR);
    int crc = htonl(69);
    write(bc->active_file, &crc, 4);
    write(bc->active_file, &now, 4);
    write(bc->active_file, &key_len_be, 2);
    write(bc->active_file, &val_len_be, 4);
    write(bc->active_file, kv->key->key, kv->key->key_len);
    write(bc->active_file, kv->val->val, kv->val->val_len);
    Metadata m = {
        .file_id = bc->max_id, 
        .value_sz = kv->val->val_len, 
        .offset=offset, 
        .timestamp=now
    };
    keydir_put(kv->key->key, kv->key->key_len, m);
    return 0;
}

void bitcask_close(Bitcask *bc) {
    if (!bc) return;
    free(bc->directory);
    close(bc->active_file);
    free(bc->active_path);
    printf("Bitcask closed\n");
    free(bc);
}
