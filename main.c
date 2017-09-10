//Name: chanmin park    ID: cs20140584

#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

//cache parameters
typedef struct{
    int s;
    int S; //after calculating the power of 2
    int E;
    int b;
    int B; //after calculating the power of 2
} cachePar;

//structure of line.
typedef struct{
    int valid; //valid bit
    long tag; //tag bits
    int used; //for implementing the LRU
} line_t;

typedef struct{
    line_t *lines;
} set_t;

typedef struct{
    set_t *sets;
} cache_t;

typedef struct{
    long tag;
    int set;
    int block;
}address_t;

//divide the block address into it's appropriate bits.
void divide_address(unsigned address_orig, cachePar* cache, address_t* address)
{
    address->block = (cache->B - 1) & address_orig;
    address->set = (cache->S - 1) & (address_orig >> cache->b);
    address->tag = address_orig >> ((cache->b) + (cache->s));
}

cache_t* create_cache(cachePar* cache_par)
{
    cache_t* cache = (cache_t*) malloc(sizeof(cache_t)); //setting the pointer of cache.
    cache->sets = (set_t*) malloc(sizeof(set_t) * cache_par->S);
    for(int i=0;i<cache_par->S;i++){ //for all sets, we create space for lines.
        cache->sets[i].lines = (line_t*) malloc(sizeof(line_t) * cache_par->E);
    }
    return cache;
}

//evicts the memory that accessed when loading and storing.
int evict_cache(address_t* address, cache_t* cache, cachePar* cache_par, int* eviction_count)
{
    int max_used = 0;
    int line_num = 0;
    for(int i=0; i<cache_par->E; i++){
        if(cache->sets[address->set].lines[i].valid == 0) {
            return i;
        }
        if(cache->sets[address->set].lines[i].used > max_used){
            line_num = i;
            max_used = cache->sets[address->set].lines[i].used;
        }
    }
    cache->sets[address->set].lines[line_num].valid = 0;    //empty the element
    (*eviction_count) ++;
    return line_num;
}


//this method is going to be used in the main function depending on the options.
void access_memory(unsigned address_orig, cache_t* cache, cachePar* cache_par, int* hit_count, int* miss_count, int* eviction_count)
{
    address_t addr;
    address_t* address = &addr;
    divide_address(address_orig, cache_par, address);
    int i;
    int found = 0;
    for(i=0;i<cache_par->E;i++){
        if(cache->sets[address->set].lines[i].valid == 0) continue;
        if(cache->sets[address->set].lines[i].tag == address->tag) {
            (*hit_count) ++;
            cache->sets[address->set].lines[i].used = 0;
            found = 1;
        }
        else {
            cache->sets[address->set].lines[i].used ++;
        }
    }
    if(!found) { // above for reached end
        (*miss_count) ++;
        int line_i = evict_cache(address, cache, cache_par, eviction_count);
        line_t* line = &(cache->sets[address->set].lines[line_i]);
        line->valid = 1;
        line->tag = address->tag;
        line->used = 0;
    }
}

int main(int argc, char **argv)
{
    int hc, mc, ec;
    
    int *hit_count = &hc;
    int *miss_count = &mc;
    int *eviction_count = &ec;
    
    int hits, misses, evictions;
    
    FILE *trace_input;
    
    char operation;
    unsigned address;
    int size;
    
    cachePar parse;
    cachePar *param = &parse;
    
    char *fileInput = NULL;
    
    cache_t *cache_input;
    char opt;
    
    //parsing according to cases using getopt (may need to declare variable 'opt'
    while ((opt = getopt(argc, argv, "s:E:b:t:")) != -1) {
        switch (opt) {
            case 's':
                param->s = atoi(optarg);
                break;
            case 'E':
                param->E = atoi(optarg);
                break;
            case 'b':
                param->b = atoi(optarg);
                break;
            case 't':
                fileInput = optarg;
                break;
            default:
                printf("\n");
                break;
        }
    }
    
    trace_input = fopen(fileInput, "r");
    
    param->S = 0x01 << (param->s);
    param->B = 0x01 << (param->b);
    
    *hit_count = 0;
    *miss_count = 0;
    *eviction_count = 0;
    
    cache_input = create_cache(param);
    
    if (trace_input != NULL) {
        while (fscanf(trace_input, " %c %x,%d", &operation, &address, &size) == 3) {
            switch (operation) {
                case 'L':
                    access_memory(address, cache_input, param, hit_count, miss_count, eviction_count);
                    break;
                case 'S':
                    access_memory(address, cache_input, param, hit_count, miss_count, eviction_count);
                    break;
                case 'M':
                    access_memory(address, cache_input, param, hit_count, miss_count, eviction_count);
                    access_memory(address, cache_input, param, hit_count, miss_count, eviction_count);
                    break;
                default:
                    break;
            } //end switch
        }
    }
    
    hits = *hit_count;
    misses = *miss_count;
    evictions = *eviction_count;
    
    free(cache_input);
    //final part: prints the output
    printSummary(hits, misses, evictions);
    fclose(trace_input);
    
    return 0;
}
