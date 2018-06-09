#include <gxx/print.h>

#include <g1/tower.h>
#include <g1/packet.h>
#include <g1/gateway.h>
#include <g1/gates/testgate.h>

#include <gxx/trace.h>

g1::testgate tgate;

int main() {
	GXX_TRACE_SIMPLE();
	g1::link_gate(&tgate, 99);

	g1::packet_header* block = g1::create_block(1, 10);
	g1::packet* pack = g1::create_packet(nullptr, block);	

	pack->set_type(9);
	pack->push_addr(99);

	memcpy(pack->dataptr(), "HelloWorld", 10);

	gxx::print_dump(block, block->flen);

	g1::transport(pack);
}