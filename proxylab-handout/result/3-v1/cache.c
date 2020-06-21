#include "cache.h"

void init_cache()
{
    for (int i = 0; i < CACHE_BLOCK_NUM; i++) {
        cache[i].time = 0;
    }
    time_cache = 0;
}

int read_cache(char *url, int fd)
{
printf("<%s>\n", __FUNCTION__);
    for (int i = 0; i < CACHE_BLOCK_NUM; i++)
    {
        if (cache[i].time && !strcmp(url, cache[i].url)) {
            Rio_writen(fd, cache[i].data, cache[i].len);
            return 1;
        }
    }
    return 0;
}

void write_cache(char *url, char *data, int len)
{
printf("<%s>\n", __FUNCTION__);
    if (len > MAX_OBJECT_SIZE)
        return;

    for (int i = 0; i < CACHE_BLOCK_NUM; i++) {
        if (!cache[i].time) {
            strcpy(cache[i].url, url);
            memcpy(cache[i].data, data, len);
            cache[i].len = len;
            cache[i].time = ++time_cache;
            return;
        }
    }

    int min_index = -1;
    int min_time = INT32_MAX;
    for (int i = 0; i < CACHE_BLOCK_NUM; i++) {
        if (cache[i].time < min_time) {
            min_index = i;
            min_time = cache[i].time;
        }
    }

    strcpy(cache[min_index].url, url);
    memcpy(cache[min_index].data, data, len);
    cache[min_index].len = len;
    cache[min_index].time = ++time_cache;
    return;
}