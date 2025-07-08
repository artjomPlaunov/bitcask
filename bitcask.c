#include "include_all.h"

#define DATA ".data"
#define ACTIVE ".active"

struct Bitcask {
    char *directory;
    char *active_path;
    uint32_t max_id;
    int active_file;
    Keydir *keydir;
    uint32_t MAX_SIZE;
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
    size_t len = snprintf(NULL, 0, "%s/%s%06d", bc->directory, DATA, id);
    char *output = malloc(len+1);
    if (!output) return NULL;
    snprintf(output, len+1, "%s/%s%06d", bc->directory, DATA, id);
    return output;
}

// void reconstruct_keydir(Bitcask *bc) {
//     Keydir *kd = bc->keydir;
//     DIR *dir = opendir(bc->directory);
//     struct dirent *entry;
// }

Bitcask *bc_open(const char *directory, uint32_t max_size) {
    
    // note -- should I implement some max directory length check here,
    // since user may pass in a non-null terminated string? 
    struct stat sb;
    // Check if path exists 
    if (stat(directory, &sb) != 0) {
        fprintf(stderr, "bc_open: cannot stat '%s': %s\n", 
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
        bc_close(bc);
        return NULL;
    }
    int fd = open(active_path, O_RDWR | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        bc_close(bc);
        return NULL;
    }
    bc->active_file = fd;
    bc->active_path = active_path;
    bc->max_id = get_next_file_id(bc->directory);
    bc->keydir = keydir_create();
    bc->MAX_SIZE = max_size;
    //reconstruct_keydir(bc);    
    printf("Bitcask opened at: %s\n", directory);
    return bc;
}

// Return -1 is error.
// Return 0 is no error. Result stored in val. 
// If key is not present, val is unchanged (user should pass in NULL.)
int bc_get(Bitcask *bc, Key *key, Value *val) {
    Metadata *m = keydir_get(bc->keydir, key->key, key->key_len);
    uint64_t offset = 4 + 4 + 2 + 4 + key->key_len;
    int fd = -1;
    int is_old_file = 0;
    char *data_path;
    // if file_id is max_id, read from active file. 
    if (m->file_id == bc->max_id) {
        fd = bc->active_file;
    // otherwise read from immutable data file. 
    } else { 
        is_old_file = 1;
        data_path = get_data_path(bc, m->file_id);
        fd = open(data_path, O_RDONLY);
        free(data_path);
    }
    lseek(fd, (m->offset) + offset, SEEK_SET);
    val->val_len = m->val_len;
    val->val = malloc(val->val_len);
    
    ssize_t bytes_read = read(fd, val->val, val->val_len);
    if (bytes_read == -1) {
        perror("read failed");
        return -1;
    }
    // if we read from immutable data file, close fd. 
    if (is_old_file) {
        close(fd);
    }
    return 0;
}

int bc_put(Bitcask *bc, Kv* kv) {
    uint8_t *buf;
    struct stat st;
    time_t now = time(NULL);
    time_t now_be = htonl(now);
    uint64_t offset = lseek(bc->active_file, 0, SEEK_CUR);
    int fd = bc->active_file;
    
    if (fstat(fd, &st) == -1) {
        perror("fstat failed");
        return -1; 
    }
    off_t cur_size = st.st_size;
    uint32_t len = kv_serialize(kv, &buf, now_be);

    // If the .active file is full, make it an immutable 
    // .data file and reset .active to an empty file. 
    if (cur_size + len > bc->MAX_SIZE) {
        // rename .active to .dataN, for next N.
        char* data_path = get_data_path(bc, bc->max_id);
        rename(bc->active_path, data_path);
        free(data_path);
        close(fd);
        // reopen .active
        fd = open(bc->active_path, O_RDWR | O_CREAT | O_APPEND | O_TRUNC, 0644);
        if (fd == -1) {
            perror("open failed");
            return -1;
        }
        bc->active_file = fd;
        offset = 0;
        bc->max_id += 1;
    }
    // In either case, fd is pointing to the current .active file.
    write(fd, buf, len);
    free(buf);
    Metadata m = {
        .file_id = bc->max_id, 
        .val_len = kv->val->val_len, 
        .offset=offset, 
        .timestamp=now
    };
    keydir_put(bc->keydir, kv->key->key, kv->key->key_len, m);
    return 0;
}

void bc_print_keydir(Bitcask *bc) {
    keydir_print(bc->keydir);
}

void bc_close(Bitcask *bc) {
    if (!bc) return;
    // rename .active to immutable .data file.
    char* data_path = get_data_path(bc, bc->max_id);
    rename(bc->active_path, data_path);
    free(data_path);
    free(bc->directory);
    close(bc->active_file);
    free(bc->active_path);
    keydir_close(bc->keydir);
    printf("Bitcask closed\n");
    free(bc);
}
