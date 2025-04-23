#include "cachelab.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csim.h"
#include <getopt.h>
#include <limits.h>
#include <math.h>

void usage()
{
    printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file> -q <policy>\n");
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    printf("  -q <policy>  Replacement policy (LRU or 2Q)\n");
    printf("\n");
    printf("Examples:\n");
    printf("  linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace -q LRU\n");
    printf("  linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace -q 2Q\n");
    exit(1);
}

// parse command line and get the parameters
void parseline(int argc, char **argv)
{
    int opt;
    int num = 0;
    while ((opt = getopt(argc, argv, "hvs:E:b:t:q:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            usage();
            break;
        case 'v':
            verbose = 1;
            break;
        case 's':
            num = atoi(optarg);
            if (num == 0 && optarg[0] != '0')
            {
                printf("./csim: Missing required command line argument\n");
                usage();
            }
            numSet = num;
            break;
        case 'E':
            num = atoi(optarg);
            if (num == 0 && optarg[0] != '0')
            {
                printf("./csim: Missing required command line argument\n");
                usage();
            }
            associativity = num;
            break;
        case 'b':
            num = atoi(optarg);
            if (num == 0 && optarg[0] != '0')
            {
                printf("./csim: Missing required command line argument\n");
                usage();
            }
            blockSize = num;
            break;
        case 't':
            strcpy(filePath, optarg);
            break;
        case 'q':
            if (strcmp(optarg, "LRU") == 0 || strcmp(optarg, "2Q") == 0)
            {
                if (strcmp(optarg, "LRU") == 0)
                    policy = 0;
                else
                    policy = 1;
            }
            else
            {
                fprintf(stderr, "Invalid replacement policy: %s\n", optarg);
                exit(1);
            }
            break;
        case ':':
            printf("./csim: Missing required command line argument\n");
            usage();
            break;
        case '?':
            usage();
            break;
        default:
            printf("getopt error");
            exit(1);
            break;
        }
    }
}

set* create_cache()
{
    set* c=(set*)malloc(sizeof(set)*numSet);//s个set
    for(int i=0;i<numSet;i++)
    {
        c[i]=(set)malloc(sizeof(line)*associativity); //e个cacheline
        for(int j=0;j<associativity;j++)
        {
            c[i][j].valid=-1;
            c[i][j].tag=-1;
            c[i][j].last_visited_time=-1;
        }
    }
    return c;
}

setnode* create_2q_cache()
{
    setnode* cacheq=(setnode*)malloc(sizeof(setnode)*numSet);
    for(int i=0;i<numSet;i++)
    {
        cacheq[i].a1=create_queue();
        cacheq[i].am=create_queue();
    }
    return cacheq;
}
void lrusimulate(unsigned long address,int is_m)
{
    //printf("%ld ",address);
    int setindex=(address/blockSize)%numSet;
    int tag=address/(blockSize*numSet);
    //printf("%d %d ",setindex,tag);
    set cset=cache[setindex];
    int lru_pos=0,lru_time=cset[0].last_visited_time;

    for(int i=0;i<associativity;i++)
    {
        if(cset[i].tag==tag)
        {
            hit++;
            if(is_m) hit++;
            cset[i].last_visited_time=timecount;
            if(verbose)
            {
                if(is_m) printf("hit hit\n");
                else printf("hit\n");
            }
            return;
        }

        if(cset[i].last_visited_time<lru_time)
        {
            lru_time=cset[i].last_visited_time;
            lru_pos=i;
        }    
    }
    miss++;
    if(is_m) hit++;
    if(lru_time!=-1) eviction++;
    if(verbose)
    {
        if(lru_time!=-1)
        {
            if(is_m)
            {
                printf("miss eviction hit\n");
            }
            else
            {
                printf("miss eviction\n");
            }
        }
        else
        {
            if(is_m) printf("miss hit\n");
            else printf("miss\n");
        }
    }

    cset[lru_pos].last_visited_time=timecount;
    cset[lru_pos].tag=tag;
    return;
}

void qsimulate(unsigned long address,int is_m)
{
    int a1_max=associativity;
    int am_max=associativity;
    int setindex=(address/blockSize)%numSet;
    int tag=address/(blockSize*numSet);
    setnode cset=cacheq[setindex];
    //printf("%d %d",setindex,tag);
    //检查am
    if(find_and_update(cset.am,tag,timecount)) 
    {
        hit++;
        if(is_m) hit++;
        if(verbose)
        {
            if(is_m) printf("hit hit\n");
            else printf("hit\n");
        }
        return;
    }
    //printf("not find in am\n");
    //检查A1-in
    if(!find_and_remove(cset.a1,tag)) //不在队列A1-in中，完全未命中
    {
        miss++;
        if(is_m)
        {
            miss++;
            if(cset.am->size == am_max)
            {
                eviction++;
                lru_replace(cset.am,tag,timecount);
            }
        }
        if(verbose)
        {
            if(is_m) 
            {
                if(cset.am->size<am_max)
                {
                    printf("miss miss\n");
                }
                else
                {
                    printf("miss miss eviction\n");
                }
            }
            else printf("miss\n");
        }
        if(cset.a1->size<a1_max) //A1-in未满，插入队列
        {
            enqueue(cset.a1,tag);
        }
        else
        {
            dequeue(cset.a1);
            enqueue(cset.a1,tag);
        }
    }
    else //在队列A1-in中且移除
    {
        miss++;
        if(is_m) hit++;
        if(cset.am->size<am_max)
        {
            enqueue(cset.am,tag);
            if(verbose)
            {
                if(is_m) printf("miss hit\n");
                else printf("miss\n");
            }
        }
        else
        {
            eviction++;
            lru_replace(cset.am,tag,timecount);
            if(verbose)
            {
                if(is_m) printf("miss eviction hit\n");
                else printf("miss eviction\n");
            }
        }
    }
    return;
}
int main(int argc, char *argv[])
{
    char operation;
    unsigned long address;
    int size;
    FILE* file;
    parseline(argc, argv);
    file=fopen(filePath,"r");
    cache=create_cache();
    cacheq=create_2q_cache();
    while(fscanf(file," %c %lx,%d\n",&operation,&address,&size)==3)
    {
        timecount++;
        if(verbose)
        {
            printf("%c %lx,%d ",operation,address,size);
        }
        switch(operation)
        {
            case 'I': continue;
            case 'M':
            {
                if(policy)
                {
                    qsimulate(address,1);
                    break;
                }
                else
                {
                    lrusimulate(address,1);
                    break;
                }
            }
            case 'L':
            {
                if(policy)
                {
                    qsimulate(address,0);
                    break;
                }
                else
                {
                    lrusimulate(address,0);
                    break;
                }
            }
            case 'S':
            {
                if(policy)
                {
                    qsimulate(address,0);
                    break;
                }
                else
                {
                    lrusimulate(address,0);
                    break;
                }
            }
        }
        
    }
    free(cache);
    free(cacheq);
    printSummary(hit, miss, eviction);
    return 0;
}