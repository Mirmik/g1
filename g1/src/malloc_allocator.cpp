#include <g1/tower.h>

void g1::utilize_block(g1::packet_header* pack) { 
	free(pack); 
}

void g1::utilize_packet(g1::packet* pack) { 
	free(pack); 
}

g1::packet_header* g1::allocate_block(uint8_t alen, uint16_t dlen) { 
	return (packet_header*)malloc(sizeof(g1::packet_header) + alen + dlen);
}

g1::packet* g1::allocate_packet() { 
	return (packet*)malloc(sizeof(g1::packet));
}