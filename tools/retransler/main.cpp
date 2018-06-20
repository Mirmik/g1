#include <gxx/print.h>
#include <gxx/print/stdprint.h>

#include <g1/tower.h>
#include <g1/packet.h>
#include <g1/gateway.h>
#include <g1/gates/testgate.h>
#include <g1/gates/selfgate.h>
#include <g1/gates/udpgate.h>
#include <g1/indexes.h>

#include <gxx/trace.h>
#include <gxx/log/target2.h>

#include <thread>

#include <readline/readline.h>
#include <readline/history.h>

#include <gxx/util/hexascii.h>
#include <gxx/util/string.h>
#include <gxx/syslock.h>
#include <gxx/sshell.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <getopt.h>
#include <csignal>

gxx::log::colored_stdout_target console_target;
g1::testgate testgate;
g1::selfgate selfgate;
g1::udpgate udpgate;

gxx::sshell sshell;

void udplistener();

int console();
int com_help(gxx::strvec&);
int com_exit(gxx::strvec&);
int com_push8(gxx::strvec&);
int com_send(gxx::strvec&);
int com_deladdr(gxx::strvec&);
int com_pushudp(gxx::strvec&);
int com_setqos(gxx::strvec&);
int com_printtower(gxx::strvec&);
int com_printin(gxx::strvec&);
int com_printout(gxx::strvec&);

void incoming_handler(g1::packet* pack);

void sigint_handler(int sig) {
	(void) sig;
	gxx::println("SIGINT requested. exit.");
	exit(0);
}

int main(int argc, char* argv[]) {
	g1::logger.link(console_target, gxx::log::level::debug);
	gxx::println("G1 Retransler");

	int udpport = 10004;

	const struct option long_options[] = {
		{"udp", required_argument, NULL, 'u'},
		{NULL,0,NULL,0}
	};

    int long_index =0;
	int opt= 0;
	while ((opt = getopt_long(argc, argv, "u", long_options, &long_index)) != -1) {
		GXX_PRINT(optarg);
		GXX_PRINT(long_index);
		GXX_PRINT(opt);
		switch (opt) {
			 case 'u': udpport = atoi(optarg); break;
			 case 0: break;
		}
	}

	g1::logger.debug("udpport: {}", udpport);

	udpgate.open(udpport);

	g1::incoming_handler = incoming_handler;
	g1::link_gate(&testgate, G1_TESTGATE);
	g1::link_gate(&udpgate, G1_UDPGATE);
	g1::link_gate(&selfgate, G1_SELFGATE);

	std::thread thr_com(console);
	g1::spin();
}

char* line_read;
char* rl_gets (const char* present) {
	if (line_read) {
		free (line_read);
		line_read = (char *)NULL;
	}

	line_read = readline (present);

	/* If the line has any text in it, save it on the history. */
	if (line_read && *line_read)
		add_history (line_read);

	return (line_read);
}

using FUNCTION = int(gxx::strvec&);

typedef struct {
	const char *name;			/* User printable name of the function. */
	FUNCTION *func;		/* Function to call to do the job. */
	const char *doc;			/* Documentation for this function.  */
} COMMAND;

COMMAND commands[] = {
	{ "help", com_help, "Display this text" },
	{ "exit", com_exit, "Exit from programm" },
	{ "push", com_push8, "Add byte to address buffer" },
	{ "pushudp", com_pushudp, "Add udp address to address buffer" },
	{ "send", com_send, "Send packet to address" },
	{ "clraddr", com_deladdr, "Clear address buffer" },
	{ "setqos", com_setqos, "Set QoS for send operation" },
	{ "printtower", com_printtower, "Print tower state" },
	{ "printin", com_printin, "debug" },
	{ "printout", com_printout, "debug" }
};

g1::address addr;
g1::QoS curqos = g1::WithoutACK;
int console() {
	__label__ __waitline__;
	while(1) {
		__waitline__:
		char* line = rl_gets(gxx::format("{}> ", gxx::hexascii_encode(addr.str)).c_str());
		gxx::strvec split = gxx::split(line, ' '); 
		
		if (split.size() == 0) goto __waitline__;

		for (COMMAND& cmd : commands) {
			if (strcmp(split[0].c_str(), cmd.name) == 0) {
				cmd.func(split);
				goto __waitline__;
			}
		}
		gxx::fprintln("Unresolved function: {}", split[0]);
	}
}

int com_help(gxx::strvec& vec) {
	for (COMMAND& cmd : commands) {
		gxx::fprintln("{} - {}", cmd.name, cmd.doc);
	}
	return (0);
}

int com_exit(gxx::strvec& vec) {
	exit(0);
	return (0);
}

int com_deladdr(gxx::strvec& vec) {
	addr.str.clear();
	return (0);
}

int com_push8(gxx::strvec& vec) {
	addr.pushuint8((char)atoi(vec[1].c_str()));
	return (0);
}

int com_pushudp(gxx::strvec& vec) {
	uint16_t port = atoi(vec[2].c_str());
	addr.pushudp(G1_UDPGATE, vec[1].c_str(), port);
	return (0);
}

int com_send(gxx::strvec& vec) {
	std::string data = vec[1];
	g1::send(addr, data, 1, curqos, 100);
	return (0);
}

int com_setqos(gxx::strvec& vec) {
	if (vec.size() < 2) { gxx::println("need specify qos [0-2]"); return -1; }
	curqos = (g1::QoS) atoi(vec[1].c_str());
	return (0);
}

int com_printtower(gxx::strvec& vec) {
	gxx::fprintln("incoming list size: {0}", g1::incoming.size());
	gxx::fprintln("outers list size: {0}", g1::outters.size());
	return (0);
}

int com_printin(gxx::strvec& vec) {
	for (auto& pack : g1::incoming) g1::print(&pack);
	return 0;
}

int com_printout(gxx::strvec& vec) {
	for (auto& pack : g1::outters) g1::print(&pack);
	return 0;
}

void incoming_handler(g1::packet* pack) {
	gxx::println("main incoming handler");
	g1::print(pack);
	g1::release(pack);
}