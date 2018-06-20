/**
@file packet.cpp
*/

#include <g1/packet.h>
#include <g1/tower.h>

#include <gxx/panic.h>

/*#include <gxx/print.h>
#include <gxx/trace.h>
#include <gxx/panic.h>


uint8_t g1::packet::gateway_index() const {
	return *((uint8_t*)(block + 1) + block->stg);
}

g1::packet_header* g1::create_block(uint8_t alen, uint16_t dlen) { 
	g1::packet_header* block = g1::allocate_block(alen, dlen);
	block -> pflag = 0;
	block -> alen = alen;
	block -> ackquant = 20;
	block -> flen = sizeof(g1::packet_header) + alen + dlen;
	block -> stg = 0;
	return block;
}
*/
g1::packet* g1::create_packet(g1::gateway* ingate, size_t addrsize, size_t datasize) { 
	g1::packet* pack = g1::allocate_packet(addrsize + datasize);
	
	pack -> header.flen = sizeof(g1::packet_header) + addrsize + datasize;
	pack -> header.alen = addrsize;
	pack -> header.ackquant = 20;
	pack -> header.pflag = 0;
	pack -> header.qos = (g1::QoS)0;
	pack -> header.stg = 0;

	dlist_init(&pack->lnk);
	pack -> ingate = ingate;
	pack -> ackcount = 0; 
	pack -> flags = 0;
	
	return pack;
}

void g1::packet_initialization(g1::packet* pack, g1::gateway* ingate) { 
	dlist_init(&pack->lnk);
	pack -> ingate = ingate;
	pack -> ackcount = 0; 
	pack -> flags = 0;
}

void g1::utilize(g1::packet* pack) {
	dlist_del(&pack->lnk);
	g1::utilize_packet(pack);
}

void g1::packet::revert_stage(void* addr1, uint8_t size1, void* addr2, uint8_t size2, uint8_t gateindex) {
	*(stageptr() + size1 + size2) = gateindex;
	memcpy(stageptr(), addr1, size1);
	memcpy(stageptr() + size1, addr2, size2);
	header.stg += 1 + size1 + size2;
}

void g1::packet::revert_stage(void* addr, uint8_t size, uint8_t gateindex) {
	gxx::panic("TODO");
}

void g1::packet::revert_stage(uint8_t gateindex) {
	*stageptr() = gateindex;
	++header.stg;
}


/*void g1::packptr::release() {
	if (ptr) {
		g1::release(ptr);
		ptr = nullptr;
	}
}

g1::packptr::~packptr() {
	release();
}*/