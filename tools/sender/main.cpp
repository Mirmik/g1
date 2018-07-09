#include <g0/core.h>
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
int srvid;
int addrsize;

gxx::log::colored_stdout_target console_target;

void incoming_handler(g1::packet* pack) {
	gxx::println("incomming handler")
}

void console_listener() {
	std::string in;
	while(1) {
		std::getline(std::cin, in);
		in += '\n';
		g0::send(0, srvid, addr, addrsize, in.data(), in.size(), (g1::QoS) 0);
	}
}

void answer_listener(g0::message* msg) {
	gxx::write(msg->data, msg->size);
}

uint16_t udpport = 9034;
std::string serial_port;
int serialfd;

int main(int argc, char* argv[]) {
	g1::logger.link(console_target, gxx::log::level::trace);
	g0::logger.link(console_target, gxx::log::level::trace);

	const struct option long_options[] = {
		{"udp", required_argument, NULL, 'u'},
		{"serial", required_argument, NULL, 's'},
		{NULL,0,NULL,0}
	};

    int long_index =0;
	int opt= 0;
	while ((opt = getopt_long(argc, argv, "us", long_options, &long_index)) != -1) {
		switch (opt) {
			case 'u': udpport = atoi(optarg); break;
			case 's': serial_port = optarg; break;
			case 0: break;
		}
	}

	g1::udpgate udpgate;
	udpgate.open(udpport);

	g1::incoming_handler = incoming_handler;
	g0::make_service(0, answer_listener);

	g1::link_gate(&udpgate, G1_UDPGATE);
	
	if (!serial_port.empty()) {
		auto ser = new serial::Serial(serial_port, 115200);
		auto* serial = new gxx::io::file(ser->fd());
		auto* serialgate = new g1::serial_gstuff_gate(serial);
		g1::link_gate(serialgate, 42);
	}
	//	addrsize = hexer(addr, 128, ".42", 3);

	//	srvid = atoi(argv[optind]);
	//	gxx::println(srvid);
	//}
	//else {
		if (argc < 3) {
			gxx::println("Usage g0send addr srvid");
			exit(-1);	
		} 

		addrsize = hexer(addr, 128, argv[optind], strlen(argv[optind]));
		if (addrsize < 0) {
			gxx::println("Wrong address format");
			exit(-1);
		}
		srvid = atoi(argv[optind+1]);
	//}


	auto thr = new std::thread(console_listener);
	g1::spin();
	//thr.join();
}

