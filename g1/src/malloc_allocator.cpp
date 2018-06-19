#include <g1/tower.h>

/*void g1::utilize_block(g1::packet_header* block) { 
	g1::logger.debug("utilize_block (ptr:{})", block);
	free(block); 
}
*/
void g1::utilize_packet(g1::packet* pack) {  
	g1::logger.debug("utilize_packet (ptr:{})", pack);
	free(pack); 
}
/*
g1::packet_header* g1::allocate_block(uint8_t alen, uint16_t dlen) {
	auto size =  sizeof(g1::packet_header) + alen + dlen;
	auto pack = (packet_header*)malloc(size);
	g1::logger.debug("allocate_block (ptr:{}, size:{})", pack, size);
	return pack;
}

g1::packet* g1::allocate_packet() {  
	auto pack = (packet*)malloc(sizeof(g1::packet));
	g1::logger.debug("allocate_packet (ptr:{}, size:{})", pack, sizeof(g1::packet));
	return pack;
}*/

g1::packet* g1::allocate_packet(size_t adlen) {
	auto size = adlen + sizeof(g1::packet);
	auto pack = (packet*) malloc(size);
	g1::logger.debug("allocate_packet (ptr:{}, size:{})", pack, size);
	return pack;
}

