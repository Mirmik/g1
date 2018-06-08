#include <g1/tower.h>
#include <g1/packet.h>
#include <g1/gateway.h>

#include <gxx/print.h>
#include <gxx/trace.h>

gxx::dlist<g1::gateway, &g1::gateway::lnk> g1::gateways;

g1::gateway* g1::find_target_gateway(const g1::packet* pack) {
	 GXX_TRACE_SIMPLE();
	uint8_t gidx = pack->gateway_index();
	for (auto& g : gateways) {
		if (g.id == gidx) return &g;
	}
	return nullptr;
}

void g1::transport(g1::packet* pack) {
	 GXX_TRACE_SIMPLE();
	g1::gateway* gate = g1::find_target_gateway(pack);
	if (gate == nullptr) return_to_tower(pack, status::WrongGate);
	else gate->send(pack);
}

void g1::return_to_tower(g1::packet* pack, g1::status sts) {
	 GXX_TRACE_SIMPLE();
	///@debug
	if (sts == g1::status::WrongGate) {
		gxx::println("WrongGate");
	}

	if (pack->bptr->qos == One) {
		utilize_block(pack->bptr);
		utilize_packet(pack);
	}

}

//void g1::print_gateways_table() {
//	gxx::println("print gateways table");
//}