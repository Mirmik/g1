#include <g1/packet.h>
#include <g1/gateway.h>
#include <g1/tower.h>

#include <gxx/print.h>
#include <gxx/trace.h>
#include <gxx/panic.h>

uint8_t g1::packet::gateway_index() const {
	return *((uint8_t*)(block + 1) + block->stg);
}

g1::packet_header* g1::create_block(uint8_t alen, uint16_t dlen) { 
	g1::packet_header* block = g1::allocate_block(alen, dlen);
	block -> alen = alen;
	block -> flen = sizeof(g1::packet_header) + alen + dlen;
	block -> stg = 0;
	return block;
}

g1::packet* g1::create_packet(g1::gateway* ingate, g1::packet_header* block) { 
	g1::packet* pack = g1::allocate_packet();
	pack -> ingate = ingate;
	pack -> block = block;
	dlist_init(&pack->lnk);
	return pack;
}

void g1::utilize(g1::packet* pack) {
	utilize_block(pack->block);
	utilize_packet(pack);
}

void g1::packet::revert_stage(void* addr1, uint8_t size1, void* addr2, uint8_t size2, uint8_t gateindex) {
	*(stageptr() + size1 + size2) = gateindex;
	memcpy(stageptr(), addr1, size1);
	memcpy(stageptr() + size1, addr2, size2);
	block->stg += 1 + size1 + size2;
}

void g1::packet::revert_stage(void* addr, uint8_t size, uint8_t gateindex) {
	gxx::panic("TODO");
}

void g1::packet::revert_stage(uint8_t gateindex) {
	*stageptr() = gateindex;
	++block->stg;
}