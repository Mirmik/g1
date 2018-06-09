#ifndef G1_SELFGATE_H
#define G1_SELFGATE_H

#include <g1/gateway.h>
#include <g1/indexes.h>

namespace g1 {
	struct selfgate : public gateway {
		void send(g1::packet* pack) override {
			pack->ingate = this;
			pack->revert_stage(G1_SELFGATE);
			g1::travell(pack);	
		}
	};
}

#endif