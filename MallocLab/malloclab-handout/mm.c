/* 
 * Andrew ID : yuanyuam
 * Name : Yuanyuan Ma
 * mm.c
 *
 * Algorithm: I have created segregated list, stored on heap, to keep track
 *            of all the free nodes. As soon as a block is freed, it is put
 *            on the free list and coalesced if needed. When allocating, 
 *            free list are checked for first fit block starting from 
 *            correct seg list for the size and iterating over other lists.
 *            If no free node found, memory is extended by chunk size.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include "mm.h"
#include "memlib.h"

/* If you want debugging output, use the following macro.  When you hand
 * in, remove the #define DEBUG line. */
#define DEBUGx
#ifdef DEBUG
# define dbg_printf(...) printf(__VA_ARGS__)
#else
# define dbg_printf(...)
#endif


/* do not change the following! */
#ifdef DRIVER
/* create aliases for driver tests */
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#endif /* def DRIVER */

/* $begin mallocmacros */
/* Basic constants and macros */
#define WSIZE                    4       /* Word and header/footer size (bytes) */
#define DSIZE                    8       /* Doubleword size (bytes) */
#define CHUNKSIZE                168     /* Extend heap by this amount (bytes) */
#define MIN_BLOCK_SIZE           (3*DSIZE)
#define SEG_LIST_SIZE_DIFF       32
#define LISTSIZE                 14


#define MAX(x, y) ((x) > (y)? (x) : (y))  

/* Word alignment */
#define ALIGNMENT 8
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(p) (((size_t)(p) + (ALIGNMENT-1)) & ~0x7)


/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc)) //line:vm:mm:pack

/* Read and write a word at address p */
#define GET(p)       (*(unsigned int *)(p))   
#define PUT(p, val)  (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)       ((char *)(bp) - WSIZE)
#define FTRP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp from heap, compute address of next and previous blocks */
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* Given block ptr bp, compter address of its previous and next allocated blocks */
#define PREV_ALLOC(bp) (GET(bp) & 0x2)
#define NEXT_ALLOC(bp) (GET(bp) & 0x4)

/* Given block ptr bp from segregated free lists, compter address of next and previous blocks */
#define NEXT_SEGBLKP(bp)            (*(char **)((char *)bp + DSIZE))
#define PREV_SEGBLKP(bp)            (*(char **)bp)

#define SEGBLKP(free_listp, index)   (*(char **)(free_listp + (index*DSIZE)))

/* Global variables */
static char *heap_listp = 0;  /* Pointer to first block */
static char *free_listp = 0;    /* Pointer to first block of seg list*/

#ifdef NEXT_FIT
    static char *rover;           /* Next fit rover */
#endif


/* Function prototypes for internal helper routines */
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void checkblock(void *bp);
static void *list_add(void *bp);
static void list_delete(void *bp);
static void *mm_coalesce(void *bp) ;
static unsigned int get_list_index(size_t asize);
static void *find_fit(size_t asize);
static int in_heap(const void *p);
static void check_cycle();
static void check_seg_pointers();
static void check_count_free_list();


/**
 * mm_init - Initialize
 * @return  -1 on error, 0 on success.
 */
int mm_init(void) {
    
    /* Create the initial empty heap */
    if ((free_listp = mem_sbrk(LISTSIZE*DSIZE)) == (void *)-1)
    return -1;
    
    /*Initialize data in seg_list to NULL*/
    for (int i = 0; i < LISTSIZE ; i++)
    {
        SEGBLKP(free_listp,i) = NULL;
    }

    heap_listp = free_listp + LISTSIZE*DSIZE;
    
    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1) 
        return -1;
    PUT(heap_listp, 0);                          /* Alignment padding */
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1)); /* Prologue header */ 
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1)); /* Prologue footer */ 
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));     /* Epilogue header */
    heap_listp += (2*WSIZE);
    

    #ifdef NEXT_FIT
        rover = heap_listp;
    #endif
    
    
//    /*Initialize data in free_listp to NULL*/
//    for (int i = 0; i < LISTSIZE ; ++i)
//    {
//        //GET_SEGI(free_listp,i) = NULL;
//        free_listp[i] = NULL;
//    }
    
   

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL) {
        dbg_printf("Error Mm_init\n");
        return -1;
    }
    return 0;
}


/**
 * malloc - Main function to allocate block of size bytes in heap
 * @param size size of heap to be allocated
 */
void *malloc (size_t size) {
    size_t asize;      /* Adjusted block size */
    size_t extendsize; /* Amount to extend heap if no fit */
    char *bp;
    
    if (heap_listp == 0){
        mm_init();
    }
    /* Ignore spurious requests */
    if (size == 0)
       return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if ( size <= DSIZE ) { 
        asize = 3*DSIZE;
    } else {
        /*add overhead and align */
        asize = ALIGN( DSIZE + size )  ;   
    }
    /* Search the free list for a fit */
    if (( bp = find_fit( asize )) != NULL ) {
        place(bp, asize);                 
        return bp;
    }
    /*Still here, we need to extend the heap*/
    extendsize = MAX( asize, CHUNKSIZE );                 
    if (( bp = extend_heap( extendsize/WSIZE )) == NULL )  
        return NULL;                         
    place( bp, asize );                       
    return bp;
}


/**
 * mm_free - Free a block
 * @param bp Block to be freed
 */
void mm_free(void *bp)
{
    if(bp == 0) 
       return;
    
    size_t size = GET_SIZE(HDRP(bp));
    if (heap_listp == 0){
       mm_init();
    }
    
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    
    
    /*Coalesce the block*/
    bp = mm_coalesce(bp);
    
    bp = list_add(bp);
}


/**
 * mm_coalesce - Coalesce the blocks to avoid fragmentation.
 *            Need to be done after every block free.
 * @param bp Block pointer to block to be coalesced
 */
static inline void *mm_coalesce(void *bp)
{   
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    
    /*The line kept, so that compiler dont complain.*/
    if(size == 0) {
        mm_checkheap(9);
    }

    if (prev_alloc && next_alloc) {               /* Case 1 */
        return bp;               
    }
    
    else if (prev_alloc && !next_alloc) {       /* Case 2 */
        /* The next block will be coalesced into bp block, 
         then delete it from list */
        list_delete(NEXT_BLKP(bp));
        
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    
    else if (!prev_alloc && next_alloc ) {       /* Case 3 */
        /* The previous block will be coalesced into bp block, 
         then delete it from list */
        list_delete(PREV_BLKP(bp));
        
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);

    }
    
    else {                                        /* Case 4 */
        /* Remove both previous and next block from thel list */
        list_delete(PREV_BLKP(bp));
        list_delete(NEXT_BLKP(bp));
        
        /* Both Previous and Next Block are not allocated */
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
        GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    
    return bp;
}


/**
 * extend_heap - Extend heap with free block and return its block pointer
 * @param words size of the heap to extend in words
 */
static inline void *extend_heap(size_t words) 
{
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE; 
    if ((long)(bp = mem_sbrk(size)) == -1)  
        return NULL;                                        

    /* Initialize free block header/footer and the epilogue header */
    PUT( HDRP( bp ), PACK( size, 0 ));         /* Free block header */
    PUT( FTRP( bp ), PACK( size, 0 ));         /* Free block footer */
    PUT( HDRP( NEXT_BLKP( bp )), PACK(0, 1)); /* New epilogue header */
    
    
    /*Coalesce the block*/
    bp = mm_coalesce(bp);
    
    bp = list_add(bp);
    return bp;
}


/**
 * place - Place block of asize bytes at start of free block bp 
 *         and split if remainder would be at least minimum block size
 * @param bp    Block where new block is to be put
 * @param asize aligned size of new block
 */
static inline void place(void *bp, size_t asize)
{
    int prev_alloc=0;
    int next_alloc=0;
    size_t csize = GET_SIZE(HDRP(bp));
    
    list_delete(bp);
    
    prev_alloc = PREV_ALLOC(PREV_BLKP(bp));
    next_alloc = NEXT_ALLOC(NEXT_BLKP(bp));
    
    if ((csize - asize) >= MIN_BLOCK_SIZE) { 
        /* Splice the etc free space */
        /* Put allocation in header, for next, previous, current
         * Bit wise or of the values, is put 
         */
        PUT(HDRP(bp), PACK(asize, (next_alloc| prev_alloc| 1)));
        PUT(FTRP(bp), PACK(asize, (next_alloc| prev_alloc| 1)));
        /* Splice the Next Block */
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize-asize, 0));
        PUT(FTRP(bp), PACK(csize-asize, 0));
        
        /*Coalesce the block*/
        bp = mm_coalesce(bp);
        
        /*Add the newly spliced block to free list*/
        bp = list_add(bp);
    } else {
        /* Do the allocation directly */
        PUT(HDRP(bp), PACK(csize, (next_alloc| prev_alloc| 1)));
        PUT(FTRP(bp), PACK(csize, (next_alloc| prev_alloc| 1)));
    }
}


/**
 * find_fit - Find the free block for the current requested size
 *                       in all the segregated lists
 * @param asize size of the free block to be searched 
 */
static inline void *find_fit (size_t asize)
{
    void *bp = 0; 
    unsigned int index = get_list_index(asize);
    
    /* index is the minimum list it should start to search for*/
    for (int i = index; i < LISTSIZE; i++) {
        for (bp = SEGBLKP(free_listp, i); (bp!=NULL) && GET_SIZE(HDRP(bp)) > 0;
         bp = NEXT_SEGBLKP(bp)) {
            if ( bp!=NULL && (asize <= GET_SIZE(HDRP(bp)))) {
                return bp;
            }
        }    
    }
    return NULL; /* No fit */
}


/**
 * checkblock - check the current block for consistency
 * @param bp Block to be checked
 */
static void checkblock(void *bp) 
{
    /**
     * Checks-
     * 1) Check for alignment
     * 2) Check for Header and Footer Match
     * 3) Check for the block to be in heap
     * 4) Check coalescing- no previous/next free blocks if current is free.
     */
    
    /*Check Block for alignment*/
    if ((size_t)bp % 8) {
       printf("ERROR: %p is not doubleword aligned\n", bp);
    }

    /*Check Block Header matching Footer*/
    if (GET(HDRP(bp)) != GET(FTRP(bp))) {
        printf("ERROR: header does not match footer\n");
        dbg_printf("**Debug Info \n");
        dbg_printf("Heap_listp = %p \n", heap_listp );
        dbg_printf("Block %p \n", bp );
    } 

    /* Check for if the block is in the heap */       
    if( !in_heap(bp)) {
        printf("ERROR: %p is not in heap \n", bp);        
    }

    /**
     * Concept : As all the blocks are iteratively checked, just checking
     *           next block for coalescing will suffice next/previous block
     *           checks.
     */    
    /* Check Coalescing with Next Block */
    if( GET_ALLOC(HDRP(bp)) ==0 && NEXT_BLKP(bp)!=NULL 
        && GET_ALLOC(HDRP(NEXT_BLKP(bp))) ==0 ) {
        printf("ERROR: %p is not coalesced with next block\n", bp);        
        exit(1);
    }

}




/**
 * check_seg_pointers - Check the seg lists for correctness of all the previous
 *                      and next pointers
 */
static void check_seg_pointers()
{
    void *bp;
    void *next;
    size_t asize;

    /* Check for all the pointers in the blocks to be correct */
    for (int i = 0; i < LISTSIZE; i++) {
        for (bp = SEGBLKP(free_listp,i); (bp!=NULL)
          &&  (GET_SIZE(HDRP(bp)) > 0);bp = NEXT_SEGBLKP(bp)) {
            next = NEXT_SEGBLKP(bp);
            asize = GET_SIZE(HDRP(bp));
            /*Check for correct bucket*/
            if( (int)get_list_index(asize) != i  ) {
                printf("ERROR: Belong to wrong seg list %p\n",bp );
            }

            if( next != NULL && PREV_SEGBLKP(next) != bp) {
                printf("ERROR: Link at block %p is broken\n",bp );
            }
        }   
    }
}


/**
 * check_cycle - Check for cycle in the free lists.
 */
static void check_cycle() 
{
    void *hare;
    void *tortoise;
    /*Implementing Tortoise and hare algo, Iterating over each seg list*/
    for (int i = 0; i < LISTSIZE; ++i) {
        hare = SEGBLKP(free_listp,i);
        tortoise = SEGBLKP(free_listp,i);

        while( tortoise != NULL && hare != NULL  ) {
            if( hare !=  NULL ) {
                hare = NEXT_SEGBLKP(hare);
            }
            if( hare != NULL ) {
                tortoise = NEXT_SEGBLKP(hare);
            }
            if( hare == tortoise ){
                /*Its a cycle .. error . */
                printf("ERROR: There is a cycle in the free_list\n");       
            }
        }
    }
}


/**
 * check_count_free_list -Check for count of free blocks, iterating over blocks 
 *                        and by going through next pointers
 */
static void check_count_free_list() 
{
    void *bp;
    unsigned int counti = 0;
    unsigned int countp = 0;
    /*Iterate over list*/   
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if(!GET_ALLOC(HDRP(bp))) {
            counti++;
        }
    }
    /* Moving free list by pointers*/
    for (int i = 0; i < LISTSIZE; i++) {
        for (bp = SEGBLKP(free_listp,i); (bp!=NULL)
          &&  (GET_SIZE(HDRP(bp)) > 0);bp = NEXT_SEGBLKP(bp)) {
            countp++;
        }   
    }

    /*If count is not matching, print error, with debug Info*/
    if(countp!=counti) {
        dbg_printf("free\n");
        dbg_printf("all\n");
    }
}



/**
 * list_add - add the specific block ptr to the head of free list
 * @param bp Pointer pointing to the block being added
 */
static void *list_add(void *bp)
{
    int index ;

    index = get_list_index(GET_SIZE(HDRP(bp)));
    /**
     * If the corresponding block is empty,
     * set its previous and next block as null
     */
    if (SEGBLKP(free_listp, index) == NULL) {
        PREV_SEGBLKP(bp) = NULL;
        NEXT_SEGBLKP(bp) = NULL;
    }
    
    else if (SEGBLKP(free_listp, index) != NULL) {
        NEXT_SEGBLKP(bp)= SEGBLKP(free_listp, index);
        PREV_SEGBLKP(SEGBLKP(free_listp, index)) = bp;
        PREV_SEGBLKP(bp) = NULL;
        
    }

    
    /**
     * Set the current pointer pointing to the head
     * of current block
     */
    (SEGBLKP(free_listp, index)) = bp;
    return bp;
}


/**:
 * list_delete - Delete a block from the free list
 * @param bp pointer pointing block deleteded 
 */
static  inline void list_delete(void *bp)
{
    int index = get_list_index(GET_SIZE(HDRP(bp)));

    void *next = NEXT_SEGBLKP(bp);
    void *prev = PREV_SEGBLKP(bp);
    
    /* Delete the head of list */
    if(bp == SEGBLKP(free_listp, index)) {
        (SEGBLKP(free_listp, index)) = next;
    }

    if(prev != NULL) {
        NEXT_SEGBLKP(prev) = next;
    }

    if(next != NULL) {
        PREV_SEGBLKP(next) = prev;
    }

    /*Clean Up task. Set next/prev pointers of bp to NULL*/
    NEXT_SEGBLKP(bp) = NULL;
    PREV_SEGBLKP(bp) = NULL;
}


/**
 * in_heap - check if the block is in the heap memory
 * @param  p pointer to the blocked to be checked
 * @return   integer flag, 1- in heap, 0- out of heap
 */
static inline int in_heap(const void *p) {
    return p <= mem_heap_hi() && p >= mem_heap_lo();
}


/**
 * mm_realloc - The function reallocates the memory, with new size
 *              by allcating a new block, and then deleting the previous
 *              block .
 * @param ptr  Block to be re-allocated
 * @param size New Size of the allocated memory
 */
void *mm_realloc ( void *ptr, size_t size )
{
    size_t oldsize;
    void *newptr;
    
    /* If size == 0 then this is just free, and we return NULL. */
    if(size == 0) {
        mm_free(ptr);
        return 0;
    }
    
    
    if(ptr == NULL) {
        return mm_malloc(size);
    }
    
    newptr = mm_malloc(size);
    
    /* If realloc() fails the original block is left untouched  */
    if(!newptr) {
        return 0;
    }
    
    /* Copy the old data. */
    oldsize = GET_SIZE(HDRP(ptr));
    if(size < oldsize)
        oldsize = size;
    memcpy(newptr, ptr, oldsize);
    
    /* Free the old block. */
    mm_free(ptr);
    return newptr;
}

/**
 * mm_checkheap - Function, to call checkheap as per verbose value
 * @param verbose value determines what things to print in checkheap
 */
void mm_checkheap(int verbose) {
    /* *
     * – Check epilogue and prologue blocks.
     * – Check each block’s address alignment.
     * – Check heap boundaries.
     * - Check each block’s header and footer: size (minimum size, alignment),
     *   previous/next allocate/free bit consistency, header
     *   and footer matching each other.
     * – Check coalescing: no two consecutive free blocks in the heap.
     */
    
    char *bp = heap_listp;
    

    if(verbose == 9) {
        return;
    }
    
    /**
     * Following condition never will be true.
     * A place function to avoid compiler compalaining
     */
    if (verbose){
        dbg_printf("Heap (%p):\n", heap_listp);
    }
    
    /* Check prologue */
    if ((GET_SIZE(HDRP(heap_listp)) != DSIZE) || !GET_ALLOC(HDRP(heap_listp))){
        printf("ERROR: Bad prologue header\n");
    }
    
    /* Check epilogue */
    if ((GET_SIZE(HDRP(heap_listp)) != 0) || !(GET_ALLOC(HDRP(heap_listp)))) {
        printf("ERROR: Bad epilogue header\n");
    }
    
    /* Check each block */
    /* boundaries, address alignment, header and footer, coalescing */
    bp = heap_listp;
    while (GET_SIZE(HDRP(bp)) > 0) {
        
        /* Address alignment */
        if ((size_t)bp % DSIZE) {
            dbg_printf("ERROR: Address is not doubleword aligned\n", bp);
        }
        
        /* Boundaries */
        if ((void *)bp > mem_heap_hi() || (void *)bp < mem_heap_lo()) {
             dbg_printf("ERROR: %p is not in heap \n", bp);
        }
        
        /* Block Header matching Footer */
        if (GET(HDRP(bp)) != GET(FTRP(bp))) {
            printf("ERROR: header does not match footer\n");
            dbg_printf("Heap_listp = %p \n", heap_listp );
            dbg_printf("Block %p \n", bp );
        }
        
        /* Coalescing with Next Block */
        if( GET_ALLOC(HDRP(bp)) == 0 && NEXT_BLKP(bp)!= NULL
           && GET_ALLOC(HDRP(NEXT_BLKP(bp))) ==0 ) {
            printf("ERROR: %p is not coalesced with next block\n", bp);
            exit(1);
        }
        
        bp = NEXT_BLKP(bp);
    }
    

    /**
     * Checks with respect to seg list
     */
    
    /*Check if all the pointers in all the free lists are aligned*/
    check_seg_pointers();
    
    
    /* Check for count of free blocks, iterating over blocks and by
     * going through next pointers*/
    check_count_free_list();
    
    /*Check for cycle in the free list*/
    check_cycle();
}


/**
 * calloc - Allocates a block of memory for an array of num elements,
 *          each of them size bytes long, and initializes all its bits to zero.
 * @param nmemb Number of Memory Elements(Array of memory)
 * @param size  Size of each Memory Element(Array of memory)
 */
void *calloc (size_t nmemb, size_t size) 
{
    size_t bytes = nmemb * size;
    void *newptr;

    newptr = malloc(bytes);
    memset(newptr, 0, bytes);

    return newptr;
}


/**
 * get_list_index - Given a asize of a block, return the its' index
*                   in the list.
 * @param - asize The size of current block
 * @return index of the block in the array
 */
static inline unsigned int get_list_index(size_t asize)
{
    if (asize <= MIN_BLOCK_SIZE) {
        return 0;
    } else if (asize <= MIN_BLOCK_SIZE << 1) {
        return 1;
    } else if (asize <= MIN_BLOCK_SIZE << 2) {
        return 2;
    } else if (asize <= MIN_BLOCK_SIZE << 3) {
        return 3;
    } else if (asize <= MIN_BLOCK_SIZE << 4) {
        return 4;
    } else if (asize <= MIN_BLOCK_SIZE << 5) {
        return 5;
    } else if (asize <= MIN_BLOCK_SIZE << 6) {
        return 6;    
    } else if (asize <= MIN_BLOCK_SIZE << 7) {
        return 7;
    } else if (asize <= MIN_BLOCK_SIZE << 8) {
        return 8;
    } else if (asize <= MIN_BLOCK_SIZE << 9) {
        return 9;
    } else if (asize <= MIN_BLOCK_SIZE << 10) {
        return 10;
    } else if (asize <= MIN_BLOCK_SIZE << 11) {
        return 11;
    } else if (asize <= MIN_BLOCK_SIZE << 12) {
        return 12;
    } else {
        return 13;
    }
}
