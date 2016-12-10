#ifndef CACHE_DEF_H_
#define CACHE_DEF_H_

#define TRUE 1
#define FALSE 0

#define KB  1024
#define MB  (1024 * 1024)

#define TAG_OFFSET  (64 - config_.tag_len)
#define BLOCK_MASK  ((uint64_t)(config_.block_size - 1))
#define SET_MASK    ((uint64_t)(config_.set_num - 1))
#define SET_OFFSET  (log2(config_.block_size))

#define SET_BITS	(log2(config_.set_num))
#define BLOCK_BITS  (log2(config_.block_size))


#define VALID 	1
#define UNVALID 0

#define WRITE_BACK 		0
#define WRITE_THROUGH	1

#define WRITE_NONALLOCATE	0
#define WRITE_ALLOCATE 		1

#define READ_OPERATION		1
#define WRITE_OPERATION		0

//Policy
#define	WRITE_BACK_ALLOCATE 1


#define DEBUG_REQUEST(addr, set_index, bytes, read_or_write, addr_tag) \
	if(read_or_write == READ_OPERATION) 	\
	{	\
		printf(" : read access addr %lld in set[%d] with %d bytes, tag=%lld\n\n", addr, set_index, bytes, addr_tag);		\
	} else {								\
		printf(" : write access addr %lld in set[%d] with %d bytes, tag=%lld\n\n", addr, set_index, bytes, addr_tag);		\
	}

#define DEBUG_CACHE_CONFIG()	\
	printf("write through[1]/back[0] is%d\n 	\
			write allocate[1]/non[0] is%d\n",	\
			config_.write_through, config_.write_allocate);	



//strategy
typedef int replace_t;
#define LRU  0
#define FIFO 1


#endif //CACHE_DEF_H_
