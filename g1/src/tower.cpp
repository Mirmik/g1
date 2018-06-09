#include <g1/tower.h>
#include <g1/packet.h>
#include <g1/gateway.h>

#include <gxx/print.h>
#include <gxx/util/hexascii.h>
#include <gxx/trace.h>

gxx::dlist<g1::gateway, &g1::gateway::lnk> g1::gateways;
gxx::log::logger g1::logger("g1");

g1::gateway* g1::find_target_gateway(const g1::packet* pack) {
	uint8_t gidx = pack->gateway_index();
	for (auto& g : gateways) {
		if (g.id == gidx) return &g;
	}
	g1::logger.warn("WrongGate: {0}", gidx);
	return nullptr;
}

void g1::travell(g1::packet* pack) {
	g1::logger.info("travelled: (type:{}, addr:{}, data:{})", pack->block->type, gxx::hexascii_encode((const uint8_t*)pack->addrptr(), pack->block->alen), pack->datasect());

	g1::gateway* gate = g1::find_target_gateway(pack);
	if (gate == nullptr) return_to_tower(pack, status::WrongGate);
	else gate->send(pack);
}

void g1::transport(g1::packet* pack) {
	pack->block->stg = 0;
	g1::travell(pack);
}

void g1::return_to_tower(g1::packet* pack, g1::status sts) {
	if (sts == g1::status::WrongGate || pack->block->qos == One) {
		utilize(pack);
		return;
	}

}

//void g1::print_gateways_table() {
//	gxx::println("print gateways table");
//}