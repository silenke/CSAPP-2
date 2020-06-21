/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */

/*
 * Segregated List，复杂合并
 * 83
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

#define FIFO

// #define CHECKHEAP(line) checkheap(line)
#define CHECKHEAP(line)

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))



#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1 << 12)

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define PACK(size, alloc) ((size) | (alloc))

#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE((char *)(bp) - DSIZE))

#define PRED(bp) ((char *)(bp))
#define SUCC(bp) ((char *)(bp) + WSIZE)

#define PRED_BLKP(bp) ((void *)(*(unsigned int *)(bp)))
#define SUCC_BLKP(bp) ((void *)(*((unsigned int *)(bp) + 1)))

#define ADDR(p) ((unsigned int)(p))

#define LISTNUM 10

static char *heap_listp = 0;
static char *heap = 0;

static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);
static void removeb(void *bp);
static void insertb(void *bp);
static void insertb2(void *bp, void *predbp, void *succbp);
static int get_root_index(size_t asize);
static void *get_root(int i);

static void printblock(void *bp);
static void checkheap(const char *line);
static void checkblock(void *bp);

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
// CHECKHEAP(__FUNCTION__);
    void *root;

    if ((heap = mem_sbrk((LISTNUM * 4 + 4) * WSIZE)) == (void *)-1)
        return -1;

    // 16,32,64,128,256,512,1024,2048,4096,...
    for (int i = 0; i < LISTNUM; i++) {
        root = get_root(i);
        PUT(root, 0);
        PUT(root + (1 * WSIZE), ADDR(root + (2 * WSIZE)));
        PUT(root + (2 * WSIZE), ADDR(root));
        PUT(root + (3 * WSIZE), 0);
    }

    heap_listp = heap + (LISTNUM * 4 * WSIZE);

    PUT(heap_listp, 0);

    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));
    heap_listp += (2 * WSIZE);

    // if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
    if (extend_heap((2 * DSIZE) / WSIZE) == NULL)
        return -1;

    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
CHECKHEAP(__FUNCTION__);
    size_t asize;
    size_t extendsize;
    char *bp;

    if (heap_listp == 0)
        mm_init();

    if (size == 0)
        return NULL;

    if (size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = DSIZE * ((DSIZE + size + (DSIZE - 1)) / DSIZE);

    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;
    place(bp, asize);

    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
CHECKHEAP(__FUNCTION__);
    if (bp == 0)
        return;

    if (heap_listp == 0)
        mm_init();

    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *bp, size_t size)
{
CHECKHEAP(__FUNCTION__);
    size_t oldsize;
    void *newbp;
    size_t copysize;

    if (size == 0) {
        mm_free(bp);
        return NULL;
    }

    if (bp == NULL) {
        return mm_malloc(size);
    }

    if ((newbp = mm_malloc(size)) == NULL) {
        return NULL;
    }

    oldsize = GET_SIZE(HDRP(bp));
    copysize = MIN(oldsize, size);
    memcpy(newbp, bp, copysize);

    mm_free(bp);

    return newbp;
}

static void *extend_heap(size_t words)
{
CHECKHEAP(__FUNCTION__);
    char *bp;
    size_t size;

    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    return coalesce(bp);
}

static void *coalesce(void *bp)
{
CHECKHEAP(__FUNCTION__);
    void *prevbp = PREV_BLKP(bp);
    void *nextbp = NEXT_BLKP(bp);

    size_t prev_alloc = GET_ALLOC(HDRP(prevbp));
    size_t next_alloc = GET_ALLOC(HDRP(nextbp));
    size_t prev_size = GET_SIZE(HDRP(prevbp));
    size_t next_size = GET_SIZE(HDRP(nextbp));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {
        insertb(bp);
    }

    else if (prev_alloc && !next_alloc) {
        size += next_size;
        removeb(nextbp);
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        insertb(bp);
    }

    else if (!prev_alloc && next_alloc) {
        size += prev_size;
        bp = prevbp;
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));

        if (get_root_index(size) == get_root_index(prev_size)) {
            removeb(bp);
            insertb(bp);
        }
    }

    else {
        size += GET_SIZE(HDRP(prevbp)) + GET_SIZE(HDRP(nextbp));
        removeb(nextbp);
        bp = prevbp;
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));

        if (get_root_index(size) == get_root_index(prev_size)) {
            removeb(bp);
            insertb(bp);
        }
    }

    return bp;
}

static void *find_fit(size_t asize)
{
CHECKHEAP(__FUNCTION__);
    int i;
    void *bp;

    for (i = get_root_index(asize); i < LISTNUM; i++) {
        for (bp = SUCC_BLKP(get_root(i)); SUCC_BLKP(bp) != 0; bp = SUCC_BLKP(bp)) {
            if (GET_SIZE(HDRP(bp)) >= asize)
                return bp;
        }
    }
    return NULL;
}

static void place(void *bp, size_t asize)
{
CHECKHEAP(__FUNCTION__);
    size_t csize = GET_SIZE(HDRP(bp));
    // void *predbp = PRED_BLKP(bp);
    // void *succbp = SUCC_BLKP(bp);

    if ((csize - asize) < (2 * DSIZE)) {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
        removeb(bp);
    }
    else {
        removeb(bp);
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize - asize, 0));
        PUT(FTRP(bp), PACK(csize - asize, 0));
        insertb(bp);
    }
}

static void removeb(void *bp)
{
    void *predbp = PRED_BLKP(bp);
    void *succbp = SUCC_BLKP(bp);

    PUT(SUCC(predbp), ADDR(succbp));
    PUT(PRED(succbp), ADDR(predbp));
}

/*
 * insertb - 在头结点后插入 bp
 */

static void insertb(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));
    void *root = get_root(get_root_index(size));
    void *succbp = SUCC_BLKP(root);

    PUT(SUCC(root), ADDR(bp));
    PUT(PRED(bp), ADDR(root));
    PUT(SUCC(bp), ADDR(succbp));
    PUT(PRED(succbp), ADDR(bp));
}

/*
 * insertb2 - 在两个节点中插入 bp
 */

static void insertb2(void *bp, void *predbp, void * succbp)
{
    PUT(SUCC(predbp), ADDR(bp));
    PUT(PRED(bp), ADDR(predbp));
    PUT(SUCC(bp), ADDR(succbp));
    PUT(PRED(succbp), ADDR(bp));
}

static int get_root_index(size_t asize)
{
    int i;

    if (asize <= 16) i = 0;
    else if (asize <= 32) i = 1;
    else if (asize <= 64) i = 2;
    else if (asize <= 128) i = 3;
    else if (asize <= 256) i = 4;
    else if (asize <= 512) i = 5;
    else if (asize <= 1024) i = 6;
    else if (asize <= 2048) i = 7;
    else if (asize <= 4096) i = 8;
    else i = 9;

    return i;
}

static void *get_root(int i)
{
    return (void *)(heap + (i * 4 * WSIZE));
}

static void printblock(void *bp) 
{
    size_t hsize, halloc, fsize, falloc;

    hsize = GET_SIZE(HDRP(bp));
    halloc = GET_ALLOC(HDRP(bp));  
    fsize = GET_SIZE(FTRP(bp));
    falloc = GET_ALLOC(FTRP(bp));  

    if (hsize == 0) {
        printf("%p: EOL\n", bp);
        return;
    }

    void *predbp = PRED_BLKP(bp);
    void *succbp = SUCC_BLKP(bp);

    if (halloc) {
    printf("%p: header: [%u:%c] footer: [%u:%c]\n", bp, 
           hsize, (halloc ? 'a' : 'f'), 
           fsize, (falloc ? 'a' : 'f')); 
    }
    else {
    printf("%p: header: [%u:%c] footer: [%u:%c] %p %p\n", bp, 
           hsize, (halloc ? 'a' : 'f'), 
           fsize, (falloc ? 'a' : 'f'),
           predbp, succbp); 
    }
}

static void checkblock(void *bp) 
{
    if ((size_t)bp % 8)
        printf("Error: %p is not doubleword aligned\n", bp);
    if (GET(HDRP(bp)) != GET(FTRP(bp)))
        printf("Error: header does not match footer\n");
}

/* 
 * checkheap - Minimal check of the heap for consistency 
 */
void checkheap(const char *line) 
{
    int verbose = 1;

    char *bp = heap_listp;

    if (verbose) {
        printf("Heap (%p):\n", heap);
        // printf("root (%p) rear (%p)\n", root, root + DSIZE);
    }

    if ((GET_SIZE(HDRP(heap_listp)) != DSIZE) || !GET_ALLOC(HDRP(heap_listp)))
        printf("Bad prologue header\n");
    checkblock(heap_listp);

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (verbose)
            printblock(bp);
        checkblock(bp);
    }

    if (verbose)
        printblock(bp);
    if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp))))
        printf("Bad epilogue header\n");

    if (verbose)
    printf("\nfunction: %s\n\n", line);
}
