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
#include <time.h>
#include "keydir.h"
#include "kv.h"

#define DATA ".data"
#define ACTIVE ".active"
#define MAX_SIZE 40

struct Bitcask {
    char *directory;
    char *active_path;
    uint32_t max_id;
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
        if (strncmp(name, DATA, strlen(DATA)) == 0) {
            const char *start = name + strlen(DATA);
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

char* get_data_path(Bitcask *bc, int id) {
    size_t len = snprintf(NULL, 0, "%s/%s%d", bc->directory, DATA, id);
    char *output = malloc(len+1);
    if (!output) return NULL;
    snprintf(output, len+1, "%s/%s%d", bc->directory, DATA, id);
    return output;
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
    int fd = open(active_path, O_RDWR | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        printf("here\n");
        bitcask_close(bc);
        return NULL;
    }
    bc->active_file = fd;
    bc->active_path = active_path;
    bc->max_id = get_next_file_id(bc->directory);

    printf("Bitcask opened at: %s\n", directory);
    return bc;
}



// -1 return means error
// 0 return means no error, and val is null if not found.
int bitcask_get(Bitcask *bc, Key *key, Value *val) {
    //keydir_print();
    Metadata *m = keydir_get(key->key, key->key_len);
    uint64_t offset = 4 + 4 + 2 + 4 + key->key_len;
    int fd = -1;
    int is_old_file = 0;
    if (m->file_id == bc->max_id) {
        fd = bc->active_file;
    } else {
        // set to true so we know to close fd later.  
        is_old_file = 1;
        char *data_path = get_data_path(bc, m->file_id);
        fd = open(data_path, O_RDONLY);
    }
    lseek(fd, (m->offset) + offset, SEEK_SET);
    val->val_len = m->value_sz;
    val->val = malloc(val->val_len);
    
    ssize_t bytes_read = read(fd, val->val, val->val_len);
    if (bytes_read == -1) {
        perror("read failed");
        return -1;
    }
    if (is_old_file) close(fd);
    return 0;
}

int bitcask_put(Bitcask *bc, Kv* kv) {
    time_t now = htonl(time(NULL));
    uint64_t offset = lseek(bc->active_file, 0, SEEK_CUR);
    int fd = bc->active_file;
    uint8_t *buf;
    uint32_t len = kv_serialize(kv, &buf, now);

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat failed");
        return -1; 
    }

    off_t cur_size = st.st_size;
    if (cur_size + len > MAX_SIZE) {
        char* data_path = get_data_path(bc, bc->max_id);
        printf("Rotating file: %s -> %s\n", bc->active_path, data_path);
        rename(bc->active_path, data_path);
        free(data_path);
        close(fd);
        fd = open(bc->active_path, O_RDWR | O_CREAT | O_APPEND | O_TRUNC, 0644);
        if (fd == -1) {
            perror("open failed");
            return -1;
        }
        bc->active_file = fd;
        offset = 0;
        bc->max_id += 1;
        printf("[bitcask_put] Rotated. New .active fd = %d\n", fd);

    }

    write(fd, buf, len);
    free(buf);
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
