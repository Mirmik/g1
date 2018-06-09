#ifndef G1_GATES_UDPGATE_H
#define G1_GATES_UDPGATE_H

#include <g1/tower.h>
#include <g1/gateway.h>

namespace g1 {
	struct udpgate : public gateway {
		void send(g1::packet* pack) override {
			g1::logger.debug("UdpGate: {0}", gxx::buffer(pack->dataptr(), pack->datasize()));
			g1::return_to_tower(pack, g1::status::Sended);
		}
	};
}

#endif