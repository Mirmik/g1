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
			g1::logger.debug("UdpGate: {0}", gxx::buffer(pack->dataptr(), pack->datasize()));

			uint32_t* addr = (uint32_t*)(pack->stageptr() + 1);
			uint16_t* port = (uint16_t*)(pack->stageptr() + 5);

			//g1::logger.debug("send udp datagramm addr:{}, port:{}", gxx::hexascii_encode((const uint8_t*)addr, 4), ntohs(*port));

			sock.ne_sendto(*addr, *port, (const char*)&pack->header, pack->header.flen);
			g1::return_to_tower(pack, g1::status::Sended);
		}

		void spin() {
			sock.nonblock(false);
			while(1) {
				g1::packet* pack = (g1::packet*) malloc(128 + sizeof(g1::packet) - sizeof(g1::packet_header));

				gxx::inet::netaddr in;
				int len = sock.recvfrom((char*)&pack->header, 128, &in);
				//g1::logger.info("udp input", len);

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
			g1::logger.info("udp input", len);
			
			g1::packet_initialization(block, this);

			block->revert_stage(&in.port, 2, &in.addr, 4, G1_UDPGATE);

			auto pack = block;
			block = nullptr;
			
			g1::travel(pack);
		}

		void open(int port) {
			sock.bind("0.0.0.0", port);
			sock.nonblock(true);
		}
	};

	inline g1::udpgate* make_udpgate(uint16_t port) {
		auto g = new udpgate;
		g->open(port);
		g1::link_gate(g, G1_UDPGATE);
		return g;
	}
}

#endif