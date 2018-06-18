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
		g1::packet_header* block;

		void send(g1::packet* pack) override {
			g1::logger.debug("UdpGate: {0}", gxx::buffer(pack->dataptr(), pack->datasize()));

			uint32_t* addr = (uint32_t*)(pack->stageptr() + 1);
			uint16_t* port = (uint16_t*)(pack->stageptr() + 5);

			g1::logger.debug("send udp datagramm addr:{}, port:{}", gxx::hexascii_encode((const uint8_t*)addr, 4), ntohs(*port));

			sock.ne_sendto(*addr, *port, (const char*)pack->block, pack->block->flen);
			g1::return_to_tower(pack, g1::status::Sended);
		}

		void exec_syncrecv() {
			while(1) {
				g1::packet_header* block = (g1::packet_header*) malloc(128);

				gxx::inet::netaddr in;
				int len = sock.recvfrom((char*)block, 128, &in);
				gxx::println("udp input", len);

				block = (g1::packet_header*)realloc(block, len);
				g1::packet* pack = g1::create_packet(this, block);

				pack->revert_stage(&in.port, 2, &in.addr, 4, G1_UDPGATE);

				g1::travell(pack);
			}
		}

		void noblock_exec() {
			sock.nonblock(true);
			if (block == nullptr) block = (g1::packet_header*) malloc(128);

			gxx::inet::netaddr in;
			int len = sock.recvfrom((char*)block, 128, &in);
			if (len <= 0) return;
			//while(1);

			block = (g1::packet_header*)realloc(block, len);
			g1::packet* pack = g1::create_packet(this, block);

			pack->revert_stage(&in.port, 2, &in.addr, 4, G1_UDPGATE);

			block = nullptr;
			g1::travell(pack);
		}

		void open(int port) {
			sock.bind("0.0.0.0", port);
		}
	};
}

#endif