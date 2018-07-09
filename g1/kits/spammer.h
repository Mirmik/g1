#ifndef G1_DGRAMM_SPAMER_H
#define G1_DGRAMM_SPAMER_H

#include <gxx/io/ostream.h>
#include <gxx/util/hexer.h>
#include <g1/tower.h>

namespace g1 {
	struct dgramm_spammer : public gxx::io::ostream {
		uint8_t* addr;
		size_t size;
		g1::QoS qos = g1::QoS(0);

		dgramm_spammer(uint8_t* addr, size_t size) : addr(addr), size(size) {}
		dgramm_spammer(const char* hexaddr, uint8_t* addr, size_t size) : addr(addr), size(size) {
			hexer(addr, size, hexaddr, strlen(hexaddr));
		}

		int writeData(const char* dat, size_t sz) {
			g1::send(addr, size, dat, sz, 0, qos);
		}
	};
}

#endif