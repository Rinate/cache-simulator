#include "memory.h"
#include "def.h"

void Memory::HandleRequest( uint64_t addr, int bytes, 
							int read,char *content, 
							int &time, int prefetch)
{
	stats_.access_counter ++;
	time = latency_.hit_latency + latency_.bus_latency;

	if(prefetch != FETCH) { stats_.access_time += time; }
}

