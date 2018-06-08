#include <g1/packet.h>
#include <g1/gateway.h>
#include <g1/tower.h>

#include <gxx/print.h>
#include <gxx/trace.h>

uint8_t g1::packet::gateway_index() const {
	return *((uint8_t*)(bptr + 1) + bptr->stg);
}

g1::packet_header* g1::create_block(uint8_t alen, uint16_t dlen) { 
	GXX_TRACE_SIMPLE();
	g1::packet_header* block = g1::allocate_block(alen, dlen);
	block -> alen = alen;
	block -> flen = sizeof(g1::packet_header) + alen + dlen;
	block -> stg = 0;
	return block;
}

g1::packet* g1::create_packet(g1::gateway* ingate, g1::packet_header* block) { 
	GXX_TRACE_SIMPLE();
	g1::packet* pack = g1::allocate_packet();
	pack -> ingate = ingate;
	pack -> bptr = block;
	dlist_init(&pack->lnk);
	return pack;
}