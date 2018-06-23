#ifndef G1_GATES_SERIAL_GSTUFF_H
#define G1_GATES_SERIAL_GSTUFF_H

#include <g1/tower.h>
#include <g1/gateway.h>
#include <gxx/gstuff/sender.h>
#include <gxx/gstuff/automate.h>

namespace g1 {
	struct serial_gstuff_gate : public gateway {
		gxx::gstuff::automate recver;
		gxx::gstuff::sender sender;

		g1::packet* rpack = nullptr;
		//char buffer[128];
		gxx::io::iostream* strm;


		serial_gstuff_gate(gxx::io::iostream* strm) : strm(strm), sender(*strm) {
			recver.debug_mode(false);
			recver.set_callback(gxx::make_delegate(&serial_gstuff_gate::handler, this));
		}

		void send(g1::packet* pack) override {
			g1::logger.debug("serial_gstuff_gate::send");
			//gxx::print_dump((char*)&pack->header, pack->header.flen);
			sender.start_message();
			sender.write((char*)&pack->header, pack->header.flen);
			sender.end_message();
			g1::return_to_tower(pack, g1::status::Sended);
		}

		void nonblock_onestep() override {
			if (rpack == nullptr) {
				init_recv();
			}

			char c;
			int len = strm->read(&c, 1);
			if (len == 1) {
				recver.newchar(c);
			}
		}

		void init_recv() {
			rpack = (g1::packet*) malloc(128 + sizeof(g1::packet) - sizeof(g1::packet_header));
			recver.init(gxx::buffer((char*)&rpack->header, 128));
		}

		void handler(gxx::buffer) {
			//gxx::println("handler");
			g1::packet* block = rpack;
			init_recv();

			block->revert_stage(id);

			g1::packet_initialization(block, this);
			g1::travel(block);
		}
	};
}

#endif