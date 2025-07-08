#ifndef INCLUDE_ALL_H
#define INCLUDE_ALL_H

// System includes
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <time.h>
#include <arpa/inet.h>  // for htonl, htons, ntohl, ntohs

// Project includes
#include "bitcask.h"
#include "keydir.h"
#include "kv.h"
#include "scan.h"
#include "uthash.h"

#endif // INCLUDE_ALL_H 