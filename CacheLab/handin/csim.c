/*
 * This is for Cache Lab - Part A
 * Author:  Yuanyuan Ma, yuanyuam@andrew.cmu.edu
 */

#include "cachelab.h"
#include <stdio.h>
#include <math.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

const int size_upper = sizeof(long) * 8;

/*
 * Cache data structure.
 */
typedef struct{
    int valid;
    unsigned long long tag;
    int access_time; 
}line;

/*
 * Cache set structure.
 */
typedef struct{
    line *cache_lines;
    int access_time;
}set;

/*
 * Cache line structure.
 */
typedef struct{
    set *cache_sets;
    int exp;
    int num_set;
    int num_line;
    int size_block;
}cache;

/*
 * Input argumets structure.
 */
typedef struct{
    int exp;
    int num_set;
    int num_line;
    int size_block;
    char *trace;
    int flag_usage;
    int flag_trace;
}arguments;

/*
 * Statistic result structure.
 */
typedef struct{
    int hits;
    int misses;
    int evictions;
}stat;

/*
 * cache constructor.
 * This method is used to initialize one cache.
 */
cache* buildCache (int s, int E, int b) {
    cache *cache_next = (cache*) malloc(sizeof(cache)); 
    cache_next->exp = s;
    cache_next->num_set = pow(2.0, s);
    cache_next->num_line = E;
    cache_next->size_block = b;

    /*
     * populate the sets
     */
    cache_next->cache_sets = (set*) malloc(sizeof(set) * cache_next->num_set);

    int i;
    for (i = 0; i < cache_next->num_set; i++) {
        set *set_temp = &(cache_next->cache_sets[i]);
        set_temp->cache_lines = (line*) malloc(sizeof(line) * cache_next->num_line);
    
        int j;
        for (j = 0; j < cache_next->num_line; j++) {
            line *line_temp = &(set_temp->cache_lines[j]);
            line_temp->valid = 0;
            line_temp->tag = 0;
            line_temp->access_time = 0;
        }
    }

    return cache_next;    
}

/*
 * getter for tag
 */
unsigned long long getTag (unsigned long long address, int s, int b) {
   unsigned long long tag = address >> (s + b);
   return tag;
}

/*
 * getter for set
 */
unsigned long long getSet (unsigned long long address, int s, int b) {
    return (address << (64 - s - b)) >> (64 -s);
}

/*
 * return 1 if hits else return 0
 */
int isHit (cache *theCache, unsigned long setIndex, unsigned long long tag) {
    set *targetSet = &(theCache->cache_sets[setIndex]);
    targetSet->access_time++;    
    int i; 
    int num_line = theCache->num_line;
    for (i = 0; i < num_line; i++) {
        line *temp = &(targetSet->cache_lines[i]);
        if (temp->valid && temp->tag == tag) {
            temp->access_time = targetSet->access_time;
            return 1;
        }
    }

    return 0;
}

/*
 * This method is used to check the current cache is full or not.
 * return 1 if the set is full else return 0
 */
int isFull (cache *theCache, unsigned long setIndex, unsigned long long tag) {
    set *targetSet = &(theCache->cache_sets[setIndex]);
    targetSet->access_time++;  
    int i;
    int num_line = theCache->num_line;
    for (i = 0; i < num_line; i++) {
        line *temp = &(targetSet->cache_lines[i]);
        if (!temp->valid) {
            temp->valid = 1;
            temp->tag = tag;
            temp->access_time = targetSet->access_time;
            return 0;
        }
    }
    return 1;
}

/*
 * This method is used to evict the oldest cache line when the cache is full.
 */
void  evict (cache *theCache, unsigned long setIndex, unsigned long long tag) {
    set *targetSet = &(theCache->cache_sets[setIndex]);
    targetSet->access_time++;
    int i;
    int num_line = theCache->num_line;
    int min = targetSet->cache_lines[0].access_time;
    line *line_evicted = &(targetSet->cache_lines[0]);
    line *temp;

    //find the oldest cache line
    for(i = 1; i < num_line; i++) {
        temp = &(targetSet->cache_lines[i]);
        if (temp->access_time < min) {
            min = temp->access_time;
            line_evicted = temp;
        }
    }

    line_evicted->valid = 1;
    line_evicted->tag = tag;
    line_evicted->access_time = targetSet->access_time;
}

/*
 * This method simulates the cache.
 * return the statistic result.
 */
stat cacheSimulator (unsigned long long address, cache *theCache, stat result) {

    unsigned long long tag = getTag (address, theCache->exp, theCache->size_block);
    unsigned long long set_index = getSet (address, theCache->exp, theCache->size_block);
    
    // if hit
    if (isHit(theCache, set_index, tag)) {
	printf("hit: %lld, %llx\n", set_index, tag);
        result.hits++; 
        return result;
    }

    //if miss
    if (!isFull(theCache, set_index, tag)) {
	printf("miss: %lld, %llx\n", set_index, tag);
        result.misses++;
        return result;     
    }

    // if evict
    evict(theCache, set_index, tag);
    printf("evict: %lld, %llx\n", set_index, tag);
    result.misses++;
    result.evictions++;
    return result;
}


int main(int argc, char** argv) {
    int opt;

    cache *theCache;

    arguments args;
    args.exp = 0;
    args.num_line = 0;
    args.size_block = 0;
    args.trace = 0;
    args.flag_usage = 0;
    args.flag_trace = 0;

    stat result;
    result.hits = 0;
    result.misses = 0;
    result.evictions = 0;

    char instru;
    unsigned long long address;
    int size;

    // read the arguments from the cammand line.
    while(-1 != (opt = getopt(argc, argv, "hvs:E:b:t:"))) {
        switch (opt) {
            case 'h':
                args.flag_usage = 1;
                break;
            case 'v':
                args.flag_trace = 1;
                break;
            case 's':
                args.exp = atoi(optarg);
                if (args.exp < 0 || args.exp > size_upper) {
                    printf("s value should be in the range from 0 to word length!\n");
                }
                break;
            case 'E':
                args.num_line = atoi(optarg);
                if (args.num_line <= 0) {
                    printf("E value shold be grater than 0!\n");
                }
                break;
            case 'b':
                args.size_block = atoi(optarg);
                if (args.size_block < 0 || args.size_block > size_upper ) {
                    printf("b value shold be in the range from 0 to word length!\n");
                }
                break;
            case 't':
                args.trace = optarg;
                break;
            default:
                break;

        }
    }

    //printf("exp: %d, num_line:%d, size_block:%d,", args.exp, args.num_line, args.size_block);

    theCache = buildCache(args.exp, args.num_line, args.size_block);

    FILE *traces_file;
    traces_file = fopen(args.trace, "r");
   // printf(args.trace);
    if (traces_file != NULL) {
        while (fscanf(traces_file, " %c %llx,%d", &instru, &address, &size) > 0) {
            switch (instru) {
                case 'I':
                    break;
                case 'M':
		    printf("address is %llx\n", address);
                    result = cacheSimulator(address, theCache, result);
                    result = cacheSimulator(address, theCache, result);
                    break;
                case 'L':
		    printf("address is %llx\n", address);
                    result = cacheSimulator(address, theCache, result);
                    break;
                case 'S':
		    printf("address is %llx\n", address);
                    result = cacheSimulator(address, theCache, result);
                    break;
                default:
                    break;
            }

	    printf("\n");
        }
    }

    printSummary(result.hits, result.misses, result.evictions);
    fclose(traces_file);
    
    printf("Finished\n");


    // free the memory allocated to cache.
    int k;
    for (k = 0; k < theCache->num_set; k++){
		free(theCache->cache_sets[k].cache_lines);
    }
    free(theCache->cache_sets);
    free(theCache);

    return 0;

}


