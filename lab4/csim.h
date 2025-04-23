#pragma once
#include <stddef.h>
#include "queue.h"

#define MININT -2147483648

// cache parameters
int numSet;
int associativity;
int blockSize;
char filePath[100];
int verbose = 0;
int policy;// 0 for LRU, 1 for 2Q

int timecount=0;
typedef struct
{
    int valid;
    int tag;
    int last_visited_time;
}line;

typedef line* set;

set* cache;
setnode* cacheq;
// final results
int hit = 0, miss = 0, eviction = 0;

// will be set in getopt() function
extern char *optarg;

// you can define functions here
void usage();
set* createcache();
setnode* create_2q_cache();
void parseline(int argc, char **argv);
