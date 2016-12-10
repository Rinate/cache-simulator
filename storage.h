#ifndef CACHE_STORAGE_H_
#define CACHE_STORAGE_H_

#include <stdint.h>
#include <stdio.h>

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&); \
  void operator=(const TypeName&)


typedef struct StorageStats_ // Storage access stats
{
  int access_counter;
  int miss_num;
  int access_time; // In cycles, totol sum
  int replace_num; // Evict old lines
  int access_lower_num; // Access lower layer
  int prefetch_num; // Prefetch
  int bypass_num;
  StorageStats_()
  {
      access_counter = 0;
      miss_num = 0;
      access_time = 0; // In cycles, totol sum
      replace_num = 0; // Evict old lines
      access_lower_num = 0; // Access lower layer
      prefetch_num = 0; // Prefetch
      bypass_num = 0; 
  }
} StorageStats;



typedef struct StorageLatency_ 
{
  int hit_latency; // In cycles
  int bus_latency; // Added to each request
} StorageLatency;



class Storage 
{
 public:
  Storage() {}
  ~Storage() {};

  // Sets & Gets
  void SetStats(StorageStats ss) { stats_ = ss; }
  void GetStats(StorageStats &ss) { ss = stats_; }
  void SetLatency(StorageLatency sl) { latency_ = sl; }
  void GetLatency(StorageLatency &sl) { sl = latency_; }

  // Main access process
  // [in]  addr: access address
  // [in]  bytes: target number of bytes
  // [in]  read: 0|1 for write|read
  // [i|o] content: in|out data
  // [out] hit: 0|1 for miss|hit
  // [out] time: total access time
  virtual void HandleRequest(uint64_t addr, int bytes, int read,
                             char *content, int &time)=0;

 protected:
  StorageStats stats_;
  StorageLatency latency_;
};

#endif //CACHE_STORAGE_H_ 
