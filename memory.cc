#include "memory.h"

void Memory::HandleRequest( uint64_t addr, int bytes, 
							int read,char *content, 
							int &time)
{
	stats_.access_counter ++;
	time = latency_.hit_latency + latency_.bus_latency;
	stats_.access_time += time;
}

