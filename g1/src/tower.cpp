#include <g1/tower.h>
#include <g1/packet.h>
#include <g1/gateway.h>

#include <gxx/print.h>
#include <gxx/util/hexascii.h>
#include <gxx/trace.h>

gxx::dlist<g1::gateway, &g1::gateway::lnk> g1::gateways;
gxx::dlist<g1::packet, &g1::packet::lnk> g1::incoming;
gxx::dlist<g1::packet, &g1::packet::lnk> g1::outters;
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

void g1::release(g1::packet* pack) {
	g1::logger.debug("released by world");
	pack->released_by_world = true;
	g1::release_if_need(pack);
}

void g1::qos_release(g1::packet* pack) {
	g1::logger.debug("released by tower");
	pack->released_by_tower = true;
	g1::release_if_need(pack);
}

void g1::travell(g1::packet* pack) {
	if (pack->block->stg == pack->block->alen) {
		g1::logger.info("incoming packet");
		g1::revert_address(pack);
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
	if (gate == nullptr) g1::utilize(pack);
	else gate->send(pack);
}

void g1::transport(g1::packet* pack) {
	pack->block->stg = 0;
	g1::travell(pack);
}

void g1::quality_notify(g1::packet* pack) {
	if (pack->block->qos == g1::TargetACK || pack->block->qos == g1::BinaryACK) {
		g1::send_ack(pack);
	}

	if (pack->block->qos == g1::BinaryACK) {
		g1::incoming.move_back(*pack);
	}
}

void g1::return_to_tower(g1::packet* pack, g1::status sts) {
	if (pack->block->qos == WithoutACK) g1::utilize(pack);
	else g1::outters.move_back(*pack);
}

void g1::print(g1::packet* pack) {
	gxx::fprintln("(type:{}, addr:{}, stg:{}, data:{})", pack->block->type, gxx::hexascii_encode((const uint8_t*)pack->addrptr(), pack->block->alen), pack->block->stg, pack->datasect());
}

void g1::release_if_need(g1::packet* pack) {
	if (pack->released_by_tower && pack->released_by_world) utilize(pack);
}

void g1::revert_address(g1::packet* pack) {
	gxx::buffer addr = pack->addrsect();

	auto first = addr.begin();
	auto last = addr.end();

	while ((first != last) && (first != --last)) {
        std::iter_swap(first++, last);
    }
}

void g1::send_ack(g1::packet* pack) {
	gxx::println("send ack");
}