#include "cachelab.h"
#include "getopt.h"
#include "stdlib.h"
#include "unistd.h"
#include "string.h"
#include "stdio.h"

enum {HIT, MISS, EVICTION};

typedef struct {
    int h;
    int v;
    int s;
    int E;
    int b;
    char *t;
} Arguments;

typedef struct {
    int valid;
    int tag;
    int lru;
} CacheLine;

int lru = 0;

void getArgs(int argc, char **argv, Arguments *args) {

    int opt;
    while (-1 != (opt = getopt(argc, argv, "hvs:E:b:t:"))) {
        switch (opt) {
            case 'h':
                args->h = 1;
                break;
            case 'v':
                args->v = 1;
                break;
            case 's':
                args->s = atoi(optarg);
                break;
            case 'E':
                args->E = atoi(optarg);
                break;
            case 'b':
                args->b = atoi(optarg);
                break;
            case 't':
                // args->t = (char *)malloc(sizeof(char) * strlen(optarg));
                // strcpy(args->t, optarg);
                args->t = optarg;
                break;
            default:
                printf("wrong argument\n");
                break;
        }
    }
}

void parseTrace(int S, int E, CacheLine cache[S][E],
                Arguments args, int ans[3], long address) {

    address = address >> args.b;
    int set = address & ((1 << args.s) - 1);
    int tag = address >> args.s;

    // 找到相应的集合
    // 遍历每一行
    // 如果 valid==1 && tag==tag，numHit++
    // 否则，numMiss++
    // 如果存在 valid==0，插入
    // 如果不存在，则寻找 lru 最小的，替换，numEviction++

    CacheLine *cacheSet = cache[set];

    for (int i = 0; i < E; i++) {
        if (cacheSet[i].valid == 1 && cacheSet[i].tag == tag) {
            cacheSet[i].lru = lru++;
            ans[HIT]++;
            if (args.v)
                printf(" hit");
            return;
        }
    }

    ans[MISS]++;
    if (args.v)
        printf(" miss");

    for (int i = 0; i < E; i++) {
        if (cacheSet[i].valid == 0) {
            cacheSet[i].valid = 1;
            cacheSet[i].tag = tag;
            cacheSet[i].lru = lru++;
            return;
        }
    }

    int minIndex = 0;
    int minLru = cacheSet[0].lru;
    for (int i = 1; i < E; i++) {
        if (cacheSet[i].lru < minLru) {
            minIndex = i;
            minLru = cacheSet[i].lru;
        }
    }
    cacheSet[minIndex].tag = tag;
    cacheSet[minIndex].lru = lru++;
    ans[EVICTION]++;
    if (args.v)
        printf(" eviction");
}

int main(int argc, char **argv) {

    // 初始化参数列表
    Arguments args;
    getArgs(argc, argv, &args);

    int S = 1 << args.s;
    int E = args.E;

    CacheLine cache[S][E];
    for (int i = 0; i < S; i++) {
        for (int j = 0; j < E; j++) {
            cache[i][j].valid = 0;
            cache[i][j].tag = -1;
            cache[i][j].lru = 0;
        }
    }

    int ans[3];
    ans[HIT] = ans[MISS] = ans[EVICTION] = 0;

    FILE *fp = fopen(args.t, "r");
    char opt;
    long address;
    int size;
    while (fscanf(fp, " %c %lx,%d\n", &opt, &address, &size) != EOF) {

        if (opt == 'I')
            continue;
        if (args.v == 1)
            printf("%c %lx,%d", opt, address, size);
        parseTrace(S, E, cache, args, ans, address);
        if (opt == 'M')
            parseTrace(S, E, cache, args, ans, address);
        printf("\n");
    }

    printSummary(ans[HIT], ans[MISS], ans[EVICTION]);

    return 0;
}
