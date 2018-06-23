#ifndef G1_TESTGATE_H
#define G1_TESTGATE_H

#include <g1/tower.h>

namespace g1 {
	struct testgate : public gateway {
		void send(g1::packet* pack) override {
			gxx::println("TestGate send");
			gxx::fprintln("TestGate: {0}", gxx::buffer(pack->dataptr(), pack->datasize()));
			g1::return_to_tower(pack, g1::status::Sended);
		}
	};
}

#endif