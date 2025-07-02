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
    
    size_t path_len = strlen(directory) + 1 + strlen(ACTIVE) + 1;
    char* active_path = malloc(path_len);
    if (!active_path) {
        fprintf(stderr, "Failed to allocate memory for file path");
        bitcask_close(bc);
        return NULL;
    }
    snprintf(active_path, path_len, "%s/%s", directory, ACTIVE);
    int fd = open(active_path, O_RDWR | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        bitcask_close(bc);
        return NULL;
    }
    bc->active_file = fd;
    bc->active_path = active_path;
    printf("%s\n", active_path);

    printf("Bitcask opened at: %s\n", directory);
    printf("next file id: %d\n", get_next_file_id(directory));
    return bc;
}

int bitcask_put
(Bitcask *bc, Kv* kv) {
    uint16_t key_len_be = htons(kv->key_len);
    uint32_t val_len_be = htonl(kv->val_len);
    time_t now = htonl(time(NULL));
    int crc = htonl(69);
    write(bc->active_file, &crc, 4);
    write(bc->active_file, &now, 4);
    write(bc->active_file, &key_len_be, 2);
    write(bc->active_file, &val_len_be, 4);
    write(bc->active_file, kv->key, kv->key_len);
    write(bc->active_file, kv->val, kv->val_len);
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
