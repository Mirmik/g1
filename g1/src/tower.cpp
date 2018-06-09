#include <g1/tower.h>
#include <g1/packet.h>
#include <g1/gateway.h>

#include <gxx/print.h>
#include <gxx/util/hexascii.h>
#include <gxx/trace.h>

gxx::dlist<g1::gateway, &g1::gateway::lnk> g1::gateways;
gxx::dlist<g1::packet, &g1::packet::lnk> g1::incoming;
void (*g1::incoming_handler)(g1::packet* pack) = nullptr;
gxx::log::logger g1::logger("g1");

g1::gateway* g1::find_target_gateway(const g1::packet* pack) {
	uint8_t gidx = pack->gateway_index();
	for (auto& g : gateways) {
		if (g.id == gidx) return &g;
	}
	g1::logger.warn("WrongGate: {0}", gidx);
	return nullptr;
}

void g1::quality_notify(g1::packet* pack) {}

void g1::release(g1::packet* pack) {
	g1::logger.debug("released by world");
	if (pack->block->qos == g1::QoS::One) {
		utilize(pack);
	}
}

void g1::qos_release(g1::packet* pack) {
	g1::logger.debug("released by qos");
}

void g1::travell(g1::packet* pack) {
	if (pack->block->stg == pack->block->alen) {
		g1::logger.info("incoming packet");
		g1::incoming.move_back(*pack);
		g1::quality_notify(pack);			

		if (g1::incoming_handler) g1::incoming_handler(pack);
		else {
			g1::logger.debug("empty incoming handler");
			g1::release(pack);
		}
		return;
	}

	g1::logger.info("travelled: (type:{}, addr:{}, stg:{}, data:{})", pack->block->type, gxx::hexascii_encode((const uint8_t*)pack->addrptr(), pack->block->alen), pack->block->stg, pack->datasect());
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

void g1::print(g1::packet* pack) {
	gxx::fprintln("(type:{}, addr:{}, stg:{}, data:{})", pack->block->type, gxx::hexascii_encode((const uint8_t*)pack->addrptr(), pack->block->alen), pack->block->stg, pack->datasect());
}

//void g1::print_gateways_table() {
//	gxx::println("print gateways table");
//}