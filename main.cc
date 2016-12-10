#include <iostream>
#include <fstream>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "cache.h"
#include "memory.h"

using namespace std;


#define IS_CACHE 1
#define IS_MEM 0

const int size =  32 * KB;               //输入以byte为单位
const int block = 128;
const int associativity = 1;
const int HZ = 2000000000;

char * PATH;

void trace(Cache* level1)
{  

    FILE*fp=fopen(PATH,"r");
    char b;
    uint64_t addr;
    

    while(!feof(fp)){
        fscanf(fp,"%c %llx\n",&b,&addr);
        
        /* if(!(isdigit(addr)))
         {
         cout<<"Wrong address!"<<endl;
         return;
         }*/
     
        /*if(addr>(1<<63))   //若地址超过64位uint
        {
            cout<<"Address Overstack!"<<endl;
            return;
        }*/
        

        bool r_or_w;
        if(b == 'r')
            r_or_w = 1;
        else if(b == 'w')
            r_or_w = 0;
        else
        {
            cout<<"Wrong operation!"<<b<<addr<<endl; //若操作不是r or w
            return;
        }

        int visit_len = 1;  // 我们先假设我们访问缓存时的数据大小总是小于block的大小
        level1->visit(addr, visit_len, r_or_w);
  }

  fclose(fp);
}


//LRU实现：给每个line结构体维护一个lastvisittime（long_int）
//用constant写读入的参数；

Cache* make_cache(int cache_size,
                int block_size,
                int associativity,
                int bus_time,
                int hit_time,
                int prefetch_num,
                int replace_policy)
{
    Cache* cache;
    StorageStats stats;
    StorageLatency latency;

    latency.bus_latency = bus_time;
    latency.hit_latency = hit_time;

    cache = new Cache(cache_size, block_size, associativity,
                      replace_policy, prefetch_num);
    cache->SetStats(stats);
    cache->SetLatency(latency);

    return cache;
}

Memory* make_memory(int bus_time, int hit_time)
{
    Memory* memory;
    /****Initialization Aruguments****/    
    StorageStats stats;
    StorageLatency mem_latency;

    mem_latency.bus_latency = bus_time;
    mem_latency.hit_latency = hit_time;

    memory = new Memory();
    memory->SetStats(stats);
    memory->SetLatency(mem_latency);
    /*************************/

    return memory;
}

void set_memory_hierarchy(Memory* memory,
            Cache* l1_cache,
            Cache* l2_cache,
            Cache* l3_cache){

    if (l2_cache == NULL){
        l1_cache->SetLower(memory);
    }else{
        l1_cache->SetLower(l2_cache);
        if (l3_cache == NULL){
            l2_cache->SetLower(memory);
        }else{
            l2_cache->SetLower(l3_cache);
            l3_cache->SetLower(memory);
        }
    }
}

void print_stat(int is_cache, const StorageStats &stats)
{

    if (is_cache){
        printf("\t\tAccess Counter = %d\n\
                Miss Num = %d\n\
                Prefetch Num = %d\n\
                Bypass Num = %d\n\
                Access Time = %d\n",
                stats.access_counter,\
                stats.miss_num,\
                stats.prefetch_num,\
                stats.bypass_num,\
                stats.access_time);
    }
    // if (is_cache){
    //     printf("\tAccess Counter = %d\n\tMiss Num = %d\n\tAccess Time = %d\n\tMiss Rate =  %f\n\tReplace Num = %d\n\tAccess Lower Num = %d\n"
    //         , stats.access_counter, stats.miss_num
    //         , stats.access_time
    //         , (double(stats.miss_num)/double(stats.access_counter))
    //         , stats.replace_num, stats.access_lower_num);
    // }
    else{
        printf("\tMem Access Counter = %d\n\tMem Access Time = %d\n"
            , stats.access_counter
            , stats.access_time);
    }
}

int main(int argc, char  *argv[]) 
{

    //get Trace Name
    PATH = argv[1];
    /****Initialization Aruguments****/
    Cache* l1_cache = NULL;
    Cache* l2_cache = NULL;
    Cache* l3_cache = NULL;
    Memory* memory = NULL;

    // get arguments
    int enable_l2, enable_l3;
    
    int cs[3], bs[3], as[3], bt[3], ht[3], pn[3], rp[3];
    int bt_mem, ht_mem;

    enable_l2 = atoi(argv[2]); // 0 or 1
    enable_l3 = atoi(argv[3]); // 0 or 1
    bt_mem = atoi(argv[4]);
    ht_mem = atoi(argv[5]);
    int cnt=6; //read from argv[5]
    for (int i=0;i<3;i++){
        cs[i] = atoi(argv[cnt++]);
        bs[i] = atoi(argv[cnt++]);
        as[i] = atoi(argv[cnt++]);
        bt[i] = atoi(argv[cnt++]);
        ht[i] = atoi(argv[cnt++]);
        pn[i] = atoi(argv[cnt++]);
        rp[i] = atoi(argv[cnt++]);
    }
    //end of getting arguments

    //initialize memory and cache
    memory = make_memory(bt_mem, ht_mem);
    l1_cache = make_cache(cs[0], bs[0], as[0], bt[0], ht[0], pn[0], rp[0]);
    if (enable_l2)
        l2_cache = make_cache(cs[1], bs[1], as[1], bt[1], ht[1], pn[1], rp[1]);
    if (enable_l3)
        l3_cache = make_cache(cs[2], bs[2], as[2], bt[2], ht[2], pn[2], rp[2]);

    set_memory_hierarchy(memory, l1_cache, l2_cache, l3_cache);

    trace(l1_cache);


    //但是只考察cache的命中情况和延迟


    //Print stats

    // printf("STRACE: %s\n", PATH);

    StorageStats stats;
    cout << "--------" <<endl<<"L1 Cache Stats:"<<endl;
    l1_cache->GetStats(stats);
    print_stat(IS_CACHE, stats);

    if (enable_l2){
        cout << "--------" <<endl<<"L2 Cache Stats:"<<endl;
        l2_cache->GetStats(stats);
        print_stat(IS_CACHE, stats);
    }

    if (enable_l3){
        cout << "--------" <<endl<<"L3 Cache Stats:"<<endl;
        l3_cache->GetStats(stats);
        print_stat(IS_CACHE, stats);
    }


    cout << "--------" <<endl<<"Memory Stats:"<<endl;
    memory->GetStats(stats);
    print_stat(IS_MEM, stats);


    return 0;
}
  
