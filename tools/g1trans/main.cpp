#include <g1/tower.h>
#include <g1/indexes.h>
#include <g1/gates/udpgate.h>
#include <g1/gates/serial_gstuff.h>
#include <gxx/log/target2.h>
#include <gxx/util/hexer.h>
#include <gxx/io/file.h>
#include <gxx/serial/serial.h>

#include <gxx/print/stdprint.h>

#include <thread>

#include <iostream>
#include <getopt.h>

uint8_t addr[128];
int addrsize;

bool raw;
bool packmon;
bool sniffer;
bool echo;

gxx::log::colored_stdout_target console_target;

void incoming_handler(g1::packet* pack) {
	if (echo) {
		g1::send(pack->addrptr(), pack->header.alen, pack->dataptr(), pack->datasize());
	}

	if (packmon) {
		gxx::print("incoming: "); 
		g1::print(pack); 
		gxx::println();
	} else {
		gxx::write(pack->dataptr(), pack->datasize());
	}
	g1::release(pack);
}

void traveling_handler(g1::packet* pack) {}

void transit_handler(g1::packet* pack) {
	if (sniffer) {
		gxx::print("transit: ");
		g1::print(pack);
		gxx::println();
	}
}

void console_listener() {
	std::string in;
	while(1) {
		std::getline(std::cin, in);
		in += '\n';
		g1::send(addr, addrsize, in.data(), in.size(), 0, (g1::QoS) 0);
	}
}

uint16_t udpport = 9034;
std::string serial_port;
int serialfd;

int main(int argc, char* argv[]) {
	const struct option long_options[] = {
		{"udp", required_argument, NULL, 'u'},
		{"serial", required_argument, NULL, 'S'},
		{"sniffer", no_argument, NULL, 's'},
		{"pack", no_argument, NULL, 'v'},
		{"echo", no_argument, NULL, 'e'},
		{NULL,0,NULL,0}
	};

    int long_index =0;
	int opt= 0;
	while ((opt = getopt_long(argc, argv, "uvSse", long_options, &long_index)) != -1) {
		switch (opt) {
			case 'u': udpport = atoi(optarg); break;
			case 'S': serial_port = optarg; break;
			case 's': sniffer = true; break;
			case 'v': packmon = true; break;
			case 'e': echo = true; break;
			case 0: break;
		}
	}

	g1::udpgate udpgate;
	udpgate.open(udpport);

	g1::incoming_handler = incoming_handler;
	g1::traveling_handler = traveling_handler;
	g1::transit_handler = transit_handler;
	g1::link_gate(&udpgate, G1_UDPGATE);
	
	if (!serial_port.empty()) {
		auto ser = new serial::Serial(serial_port, 115200);
		auto* serial = new gxx::io::file(ser->fd());
		auto* serialgate = new g1::serial_gstuff_gate(serial);
		g1::link_gate(serialgate, 42);
	}

	if (optind < argc) {
		addrsize = hexer(addr, 128, argv[optind], strlen(argv[optind]));
		if (addrsize < 0) {
			gxx::println("Wrong address format");
			exit(-1);
		}	
		auto thr = new std::thread(console_listener);
		thr->detach();
	}

	g1::spin();
}

