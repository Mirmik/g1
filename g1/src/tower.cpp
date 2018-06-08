#include <g1/tower.h>
#include <g1/package.h>

gxx::dlist<gateway, &gateway::lnk> gateways;

g1::gateway* g1::find_target_gateway(const g1::packet& pack) {
	uint8_t gidx = pack.gateway_index();
	for (auto& g : gateways) {
		if (g.id == gidx) return &g;
	}
	return nullptr;
}

void g1::transport(const g1::packet& pack) {
	g1::gateway* gate = g1::find_target_gateway(pack);
	if (gate == nullptr) return_to_tower(pack, status::WrongGate);
	else gate->send(pack);
}

void g1::return_to_tower(const g1::packet& pack) {
	if (pack->bptr->qos == One) {
		utilize_block(pack->bptr);
		utilize_pack(pack);
	}

}

//void g1::print_gateways_table() {
//	gxx::println("print gateways table");
//}