#include <gxx/print.h>
#include <gxx/print/stdprint.h>

#include <g1/tower.h>
#include <g1/packet.h>
#include <g1/gateway.h>
#include <g1/gates/testgate.h>
#include <g1/gates/udpgate.h>
#include <g1/indexes.h>

#include <gxx/trace.h>
#include <gxx/log/target2.h>

#include <thread>

#include <readline/readline.h>
#include <readline/history.h>

#include <gxx/util/hexascii.h>
#include <gxx/util/string.h>
#include <gxx/sshell.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

gxx::log::colored_stdout_target console_target;
g1::testgate testgate;
g1::udpgate udpgate;

gxx::sshell sshell;

int console();
int com_help(gxx::strvec&);
int com_exit(gxx::strvec&);
int com_push8(gxx::strvec&);
int com_send(gxx::strvec&);
int com_deladdr(gxx::strvec&);
int com_pushudp(gxx::strvec&);

int main() {
	g1::logger.link(console_target, gxx::log::level::debug);
	gxx::println("G1 Retransler");

	g1::link_gate(&testgate, G1_TESTGATE);
	g1::link_gate(&udpgate, G1_UDPGATE);

	std::thread thr(console);

	thr.join();
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
	{ "clraddr", com_deladdr, "Clear address buffer" }
};

std::string addr;
int console() {
	__label__ __waitline__;
	while(1) {
		__waitline__:
		char* line = rl_gets(gxx::format("{}> ", gxx::hexascii_encode(addr)).c_str());
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
}

int com_exit(gxx::strvec& vec) {
	exit(0);
}

int com_deladdr(gxx::strvec& vec) {
	addr.clear();
}

int com_push8(gxx::strvec& vec) {
	addr.push_back((char)atoi(vec[1].c_str()));
}

int com_pushudp(gxx::strvec& vec) {
	//addr.push_back((char)atoi(vec[1].c_str()));

	uint32_t iaddr = inet_addr(vec[1].c_str());
	uint16_t port = htons(atoi(vec[2].c_str()));

	addr.push_back(G1_UDPGATE);
	addr.append((const char*)&iaddr, 4);
	addr.append((const char*)&port, 2);
}

int com_send(gxx::strvec& vec) {
	if (addr.size() == 0) {
		gxx::println("null address packet prevented");
		return -1;
	}

	std::string data = vec[1];
	auto block = g1::create_block(addr.size(), data.size());
	auto pack = g1::create_packet(nullptr, block);
	pack->set_type(1);
	memcpy(pack->addrptr(), addr.data(), addr.size());
	memcpy(pack->dataptr(), data.data(), data.size());

	g1::transport(pack);
}