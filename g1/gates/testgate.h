#ifndef G1_TESTGATE_H
#define G1_TESTGATE_H

namespace g1 {
	struct testgate : public gateway {
		void send(g1::packet* pack) override {

		}
	};
}

#endif