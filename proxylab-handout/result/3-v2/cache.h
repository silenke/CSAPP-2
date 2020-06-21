#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

#define CACHE_BLOCK_NUM 10

typedef struct {
    char url[MAXLINE];
    char data[MAX_OBJECT_SIZE];
    int len;
    int time;
    sem_t mutex;
} cacheblock;

cacheblock cache[CACHE_BLOCK_NUM];
int time_cache;

void init_cache();
int read_cache(char *url, int fd);
void write_cache(char *url, char *data, int len);