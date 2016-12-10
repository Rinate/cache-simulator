#ifndef CACHE_CACHE_H_
#define CACHE_CACHE_H_

#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <queue>
#include <iostream>
#include "storage.h"
#include "def.h"


inline int log2(int value){
  if(value == 1)
    return 0;
  else
    return 1 + log2(value>>1);
}


typedef struct CacheConfig_ 
{
  int size;
  int associativity;  
  int set_num;      // Number of cache sets=index
  
  //加入两个定义 block size&tag_len
  int block_size;
  int tag_len;

} CacheConfig;


class Line
{    
public:
    char valid;              //valid=0--unvalid block;valid=1--valid block
    char dirty;
    uint64_t tag;            //tag定义为64位int数，方便比较操作
    char* block;             //data用malloc实现动态分配大小，会不会影响取数据的时间？ 
    uint64_t counter;        //维护一个counter,用来记录最近一次使用的时间，实现LRU
    Line(int num)            //num=block size
    {           
        block = new char[num];
        valid = 0;
        dirty = 0;
    }
    ~Line()= default;
};


class Set
{
public:
  std::vector<Line> set_st;
  std::queue<int> fifo_q;  //maintain a replace queue for FIFO, record seq in set_st
  uint64_t last_hit_tag;
  Set(int line_num,int block_ssize)
  {   //构造函数
    last_hit_tag = 0;

    for(int k = 0;k < line_num; k++)
    {
      Line* tmp = new Line(block_ssize);
      set_st.push_back(*tmp);
    }
  } 
};



class Cache: public Storage {
 public:
  Cache(int size_, int block_, int ass_,
        int replacement, int prefetch_num)
  { 

    config_.size = size_;               //这里默认输入以byte为单位
    config_.block_size = block_;
    config_.associativity = ass_;
    config_.set_num = size_ / (block_ * ass_);
    config_.tag_len = 64-log2(block_)-log2(config_.set_num);   //需保证block_, set_num 是2的幂
    
    // printf("Size:%x\n", config_.size);
    // printf("block_size:%x\n", config_.block_size);
    // printf("associativity:%x\n", config_.associativity);
    // printf("set_num:%x\n", config_.set_num);
    // printf("tag_len:%x\n", config_.tag_len);


    strategy = replacement;
    prefetch_blocks = prefetch_num; 



    for(int i = 0;i < config_.set_num;i++)
    { 

      Set *tmp_set = new Set(config_.associativity, config_.block_size);
      mycache_.push_back(*tmp_set);

    }

    
  }
  ~Cache()
  { 
    for (int i = 0; i < config_.set_num; ++i)
    {
        for (int k = 0; k < config_.associativity; ++k)
        {
            delete [] mycache_[i].set_st[k].block;
        }
    }
  }

  
  /* Inherit */
  // void SetStats(StorageStats ss) { stats_ = ss; }
  // void GetStats(StorageStats &ss) { ss = stats_; }
  // void SetLatency(StorageLatency sl) { latency_ = sl; }
  // void GetLatency(StorageLatency &sl) { sl = latency_; }

  void SetConfig(CacheConfig cc);
  void GetConfig(CacheConfig& cc);
  
  void SetLower(Storage* ll) {
    lower_ = ll; 
    // printf("LOWER %x\n", lower_);
  }   
  
  void HandleRequest(uint64_t addr, int bytes, int read_or_write,
                     char *content, int& time);

  void visit(uint64_t addr,int len, int read_or_write);
 private:
  
  int BypassDecision(uint64_t addr);  
  void BypassAlgorithm(uint64_t addr, int bytes, int read, 
                      char* content, int& time);
  
  int AccessHit(int set_index, uint64_t addr_tag, int& set_way);
  // int ReplaceDecision(int set_index, uint64_t addr_tag, int& line_offset);    //
  void ReplaceAlgorithm(int set_index,uint64_t addr, int bytes, int read_or_write,
                          char *content, int& time);
  
  inline int PrefetchDecision();
  void PrefetchAlgorithm(uint64_t addr, int& time);


  std::vector<Set> mycache_; 
  
  CacheConfig config_;

  /* Inherit */
  //StorageStats stats_;
  // StorageLatency latency_;
  
  Storage *lower_;

  replace_t strategy;
  int prefetch_blocks;


  DISALLOW_COPY_AND_ASSIGN(Cache);
};


#endif //CACHE_CACHE_H_ 
 
