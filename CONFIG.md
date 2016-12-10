# 设置思路

## 可变参数
   Cache的容量和层数

层数
    L1
    L2
    L3

容量
    L1 Cache不超过2MB
    L1: 8种设置
        16KB - 2^14 Byte
        ...
        2MB - 2^21 Byte
    L2:
        128KB 2^17 Byte
        16MB - 2^24 Byte
    L3:

        128MB - 2^27 Byte

相联度
    1 2 4 8 16

块大小
    16 32 64 128

```
#argv0   ELF NAME (don't need to feed as an argument)
argv1   trace path
# all following arguments in decimal
argv2   enable_l2 (1 or 0)
argv3   enable_l3 (1 or 0)
argv4   memory_bus_latency
argv5   memory_hit_latency
    L1 STATS:
argv6   l1_cache_size
argv7   l1_block_size
argv8   l1_associativity
argv9   l1_bus_latency
argv10  l1_hit_latency
argv11  l1_prefetch_num
argv12  l1_replace_policy
    L2 STATS:
argv13  l2_cache_size
argv14  l2_block_size
argv15  l2_associativity
argv16  l2_bus_latency
argv17  l2_hit_latency
argv18  l2_prefetch_num
argv19  l2_replace_policy
    L3 STATS:
argv16   l3_cache_size
argv17   l3_block_size
argv18   l3_associativity
argv19   l3_bus_latency
argv20   l3_hit_latency
```

Example ./sim ./trace/1.trace 1 0 0 100 32768 64 8 0 4 262144 64 8 3 5 0 0 0 0 0

        ./sim ./trace/1.trace 1 0 100 0 32768 64 8 0 4 1 1 262144 64 8 6 5 1 1 0 0 0 0 0

可用batch_tests方便的设置