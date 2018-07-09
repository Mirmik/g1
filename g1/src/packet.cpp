/**
@file packet.cpp
*/

#include <g1/packet.h>
#include <g1/tower.h>

#include <gxx/panic.h>
#include <gxx/syslock.h>

g1::packet* g1::create_packet(g1::gateway* ingate, size_t addrsize, size_t datasize) { 
	gxx::system_lock();
	g1::packet* pack = g1::allocate_packet(addrsize + datasize);
	gxx::system_unlock();
	
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
	gxx::system_lock();
	dlist_del(&pack->lnk);
	g1::utilize_packet(pack);
	gxx::system_unlock();
}

void g1::packet::revert_stage(void* addr1, uint8_t size1, void* addr2, uint8_t size2, uint8_t gateindex) {
	*(stageptr() + size1 + size2) = gateindex;
	memcpy(stageptr(), addr1, size1);
	memcpy(stageptr() + size1, addr2, size2);
	header.stg += 1 + size1 + size2;
}

void g1::packet::revert_stage(void* addr, uint8_t size, uint8_t gateindex) {
	*(stageptr() + size) = gateindex;
	memcpy(stageptr(), addr, size);
	header.stg += 1 + size;
}

void g1::packet::revert_stage(uint8_t gateindex) {
	*stageptr() = gateindex;
	++header.stg;
}