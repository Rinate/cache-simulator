#include "cache.h"
#include "def.h"
// #define DEBUG

void Cache::visit(uint64_t addr,int len,int read_or_write)
{
    char *content = new char[len];
    int time = 0;

    HandleRequest(addr, len, read_or_write, content, time);

    delete [] content;
}

void Cache::HandleRequest(uint64_t addr, int bytes, int read,
                          char *content, int &time) 
{

    // request
    stats_.access_counter ++;

    // request cost
    time += latency_.bus_latency;

    // check hit/miss cost
    time += latency_.hit_latency;



    uint64_t addr_tag = addr >> TAG_OFFSET;
    int block_offset = (int)(addr & BLOCK_MASK);    //假设都是block_size 这样的参数都是4，8 这样2的幂
    int set_index = (int)((addr >> SET_OFFSET) & SET_MASK); //在哪一个组
    int set_way = 0;    //在哪一个路


    #ifdef DEBUG
    printf("BLOCK_MASK %x\n", BLOCK_MASK);
    printf("SET_OFFSET %x\n", SET_OFFSET);
    printf("SET_MASK %x\n", SET_MASK);
    printf("ADDR %x\n", addr);
    printf("Tag, BO, SI: %x %x %x\n", addr_tag, block_offset, set_index);
    #endif


    if(!AccessHit(set_index, addr_tag, set_way))
    {
        stats_.miss_num++;


        if(BypassDecision(addr_tag, set_index))
        {
            BypassAlgorithm(addr, bytes, read, 
                            content, time);
        }
        else
        {
            ReplaceAlgorithm(set_index, addr, bytes, read,
                             content, time);
        }
        if(PrefetchDecision())
        {
            PrefetchAlgorithm(addr, time);
        }
    }
    else    // hit
    {  
        mycache_[set_index].last_hit_tag = addr_tag;
        if(read == READ_OPERATION)
        {
            memcpy(content, mycache_[set_index].set_st[set_way].block, bytes);
        }
        else
        {
            auto& line = mycache_[set_index].set_st[set_way];
        
            line.dirty = 1;
            line.tag = (addr >> TAG_OFFSET);
            memcpy(line.block, content, config_.block_size);
        }
    }
    

    stats_.access_time += time;
}

int Cache::AccessHit(int set_index, uint64_t addr_tag, int& set_way)
{
    auto& set = mycache_[set_index].set_st;    
    for (int i = 0; i < config_.associativity; ++i)
    {
        // printf("\t\tset[i].valid = %d\n\
        //         set[i].tag = %llx\n\
        //         addr_tag = %llx\n\n", 
        //         set[i].valid, set[i].tag, addr_tag);
        if(set[i].valid == VALID && set[i].tag == addr_tag)
        {
            set_way = i;    // choose the block
            set[i].counter = stats_.access_counter; //global counter
            set[i].frequency ++;  //LFU
            return TRUE;
        }
    }
    return FALSE;    
}

int Cache::BypassDecision(uint64_t addr_tag, int set_index)
{   
    const auto set_tag = mycache_[set_index].last_hit_tag;
    if(set_tag == addr_tag)     // conflict miss
    {
        return FALSE;
    }
    else    // capacity miss
    {
        stats_.bypass_num++;
        return TRUE;
    }

}

void Cache::BypassAlgorithm(uint64_t addr, int bytes, int read, 
                            char* content, int& time)
{
    int lower_time = 0;     // bypass current cache

    lower_->HandleRequest(addr, bytes, read, 
                        content, lower_time);
    
    time += lower_time;
    stats_.access_lower_num += 1;
}

void Cache::ReplaceAlgorithm(int set_index, uint64_t addr, int bytes, int read_or_write,
                          char *content, int &time)
{       
    auto& set = mycache_[set_index].set_st;
    auto& queue = mycache_[set_index].fifo_q;
    //  cache is not full
    for (int i = 0; i < config_.associativity; ++i) 
    {
        if(set[i].valid == UNVALID)
        {
                
            switch (strategy)
            {
                case LRU:
                    set[i].counter = stats_.access_counter; //找到确定的缓存块目标
                    break;

                case FIFO:
                    queue.push(i);
                    break;
                case LFU:
                    set[i].frequency = 1;
                    set[i].counter = stats_.access_counter;
                default:
                    printf("non-exist cache replacement strategy %d\n", strategy);
            }
            


            if(read_or_write == READ_OPERATION)
            {
                int lower_time = 0;

                stats_.access_lower_num ++;
                lower_->HandleRequest(addr, config_.block_size, read_or_write
                                            , set[i].block, lower_time);
                        
                set[i].dirty = 0;
                set[i].valid = VALID;
                set[i].tag = (uint64_t)(addr >> TAG_OFFSET);

                time += lower_time;
            }
            else    
            {
                //注意更新的内容大小，bytes
                set[i].valid = 1;
                set[i].tag   = (uint64_t)(addr >> TAG_OFFSET);
                set[i].dirty = 1;
                        
                memcpy(set[i].block, content, config_.block_size);

            }
            return;
        }
    }

    int cnt = 0;

    //  cache is full, find the right block to replace depending on different replace algorithm
    switch(strategy)      
    {
        case LRU:
            for (int i = 1; i < config_.associativity; ++i) 
            {
                cnt = set[cnt].counter < set[i].counter ? cnt : i;
            }
            break;

        case FIFO:
            cnt = queue.front();
            queue.pop();
            queue.push(cnt);
            break;
        case LFU:
            int least_f = 1<<31;
            for(int i = 1; i< config_.associativity;++i)
            {
                if(set[i].frequency < least_f)
                {
                    cnt = i;
                    least_f = set[i].frequency;
                }
            }
            for(int i = 1; i< config_.associativity;++i)
            {
                if(set[i].frequency == least_f)  //if equal frequency, refer to recent time
                {
                    if(set[i].counter < set[cnt].counter)
                    {
                        cnt = i;
                    }
                }
                set[i].frequency = 0;  //reset after replace takes place
            }
            set[cnt].frequency = 1;
            break;
        default:
         printf("non-exist cache replacement strategy %d\n", strategy);
    }
    set[cnt].counter = stats_.accsess_counter;
    stats_.replace_num ++;
    
    
    int lower_time = 0;
    //  write back policy
    if(set[cnt].dirty == 1 )
    {
        uint64_t tmp_addr = set[cnt].tag;
        tmp_addr = (tmp_addr << SET_BITS) | set_index;
        tmp_addr = (tmp_addr << BLOCK_BITS) ;//block 字节对齐

        
        lower_->HandleRequest(tmp_addr, config_.block_size, 
                              WRITE_OPERATION, set[cnt].block, 
                              lower_time);
        

        time += lower_time;
    }


    lower_time = 0;
    if(read_or_write == WRITE_OPERATION)
    {
            set[cnt].dirty = 1;
            set[cnt].tag = (addr >> TAG_OFFSET);

            memcpy(set[cnt].block, content, bytes);
    }
    else    // read
    {
        set[cnt].dirty = 0;
        stats_.access_lower_num ++;
        lower_->HandleRequest(addr, config_.block_size, READ_OPERATION
                            , set[cnt].block, lower_time);

        set[cnt].tag = (uint64_t)(addr >> TAG_OFFSET);

        time += lower_time;
    }

    return;            
}
        
    

inline int Cache::PrefetchDecision(){return TRUE;}

void Cache::PrefetchAlgorithm(uint64_t addr, int &time) 
{
    

    stats_.prefetch_num += prefetch_blocks;
    stats_.access_lower_num += prefetch_blocks;

    for (int i = 0; i < prefetch_blocks; ++i)
    {
        uint64_t cur_addr = addr + prefetch_blocks * config_.block_size;
        
        int lower_time = 0;
        char *content = new char[config_.block_size];


        lower_->HandleRequest(cur_addr, config_.block_size, READ_OPERATION, 
                                content, lower_time);

        // we don't count the time cost on prefetch unnessary data        

        int set_index = (int)((cur_addr >> SET_OFFSET) & SET_MASK); 
        ReplaceAlgorithm(set_index, cur_addr, config_.block_size, WRITE_OPERATION,
                             content, time);

        delete [] content;
    }
}