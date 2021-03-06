#include "cache.h"

void init_cache()
{
    for (int i = 0; i < CACHE_BLOCK_NUM; i++) {
        cache[i].time = 0;
        Sem_init(&cache[i].mutex, 0, 1);
    }
    time_cache = 0;
    readcnt = 0;
    Sem_init(&mutex_readcnt, 0, 1);
}

int read_cache(char *url, int fd)
{
printf("<%s>\n", __FUNCTION__);
    for (int i = 0; i < CACHE_BLOCK_NUM; i++)
    {
        P(&mutex_readcnt);
        readcnt++;
        if (readcnt == 1)
            P(&cache[i].mutex);
        V(&mutex_readcnt);
        if (cache[i].time && !strcmp(url, cache[i].url))
        {
            Rio_writen(fd, cache[i].data, cache[i].len);
            V(&cache[i].mutex);
            return 1;
        }
        P(&mutex_readcnt);
        readcnt--;
        if (readcnt == 0)
            V(&cache[i].mutex);
        V(&mutex_readcnt);
    }
    return 0;
}

void write_cache(char *url, char *data, int len)
{
printf("<%s>\n", __FUNCTION__);
    if (len > MAX_OBJECT_SIZE)
        return;

    for (int i = 0; i < CACHE_BLOCK_NUM; i++)
    {
        P(&cache[i].mutex);
        if (!cache[i].time) {
            strcpy(cache[i].url, url);
            memcpy(cache[i].data, data, len);
            cache[i].len = len;
            cache[i].time = ++time_cache;
            V(&cache[i].mutex);
            return;
        }
        V(&cache[i].mutex);
    }

    int min_index = -1;
    int min_time = INT32_MAX;
    for (int i = 0; i < CACHE_BLOCK_NUM; i++)
    {
        P(&cache[i].mutex);
        if (cache[i].time < min_time) {
            min_index = i;
            min_time = cache[i].time;
        }
        V(&cache[i].mutex);
    }

    P(&cache[min_index].mutex);
    strcpy(cache[min_index].url, url);
    memcpy(cache[min_index].data, data, len);
    cache[min_index].len = len;
    cache[min_index].time = ++time_cache;
    V(&cache[min_index].mutex);
    return;
}