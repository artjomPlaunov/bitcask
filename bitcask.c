#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "bitcask.h"
#include "limits.h"

#define FNAME_PREFIX ".data"
#define ACTIVE_FILE ".active"
#define MAX_SIZE 100

struct Bitcask {
    char *directory;
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
    


    printf("Bitcask opened at: %s\n", directory);
    printf("next file id: %d\n", get_next_file_id(directory));
    return bc;
}

void bitcask_close(Bitcask *bc) {
    if (!bc) return;
    free(bc->directory);
    printf("Bitcask closed\n");
    free(bc);
}
