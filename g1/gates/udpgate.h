#ifndef G1_GATES_UDPGATE_H
#define G1_GATES_UDPGATE_H

#include <g1/tower.h>
#include <g1/gateway.h>
#include <g1/indexes.h>
#include <gxx/inet/dgramm.h>
#include <gxx/util/hexascii.h>

namespace g1 {
	struct udpgate : public gateway {
		gxx::inet::udp_socket sock;
		g1::packet* block = nullptr;

		void send(g1::packet* pack) override {
			uint32_t* addr = (uint32_t*)(pack->stageptr() + 1);
			uint16_t* port = (uint16_t*)(pack->stageptr() + 5);
			sock.ne_sendto(*addr, *port, (const char*)&pack->header, pack->header.flen);
			g1::return_to_tower(pack, g1::status::Sended);
		}

		void spin() {
			sock.nonblock(false);
			while(1) {
				g1::packet* pack = (g1::packet*) malloc(128 + sizeof(g1::packet) - sizeof(g1::packet_header));
				gxx::inet::netaddr in;
				int len = sock.recvfrom((char*)&pack->header, 128, &in);
				g1::packet_initialization(pack, this);				
				pack->revert_stage(&in.port, 2, &in.addr, 4, G1_UDPGATE);
				g1::travel(pack);
			}
		}

		void nonblock_onestep() override {
			if (block == nullptr) block = (g1::packet*) malloc(128 + sizeof(g1::packet) - sizeof(g1::packet_header));

			gxx::inet::netaddr in;
			int len = sock.recvfrom((char*)&block->header, 128, &in);
			if (len <= 0) return;			
			g1::packet_initialization(block, this);

			block->revert_stage(&in.port, 2, &in.addr, 4, G1_UDPGATE);

			auto pack = block;
			block = nullptr;
			
			g1::travel(pack);
		}

		int open(int port) {
			int ret = sock.bind("0.0.0.0", port);
			if (ret >= 0) sock.nonblock(true);
			return ret;
		}

		int open() {
			sock.nonblock(true);
			return 0;
		}
	};

	/*inline g1::udpgate* make_udpgate(uint16_t port) {
		auto g = new udpgate;
		g->open(port);
		g1::link_gate(g, G1_UDPGATE);
		return g;
	}*/

	udpgate* create_udpgate(uint16_t port, uint8_t id = G1_UDPGATE);
}

#endif