#ifndef G1_TESTGATE_H
#define G1_TESTGATE_H

namespace g1 {
	struct testgate : public gateway {
		void send(g1::packet* pack) override {
			gxx::fprintln("TestGate: {0}", gxx::buffer(pack->dataptr(), pack->datasize()));
			g1::return_to_tower(pack, g1::status::Sended);
		}
	};
}

#endif