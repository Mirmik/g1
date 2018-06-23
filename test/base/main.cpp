#include <gxx/print.h>
#include <gxx/log/target2.h>

#include <g1/tower.h>
#include <g1/indexes.h>
#include <g1/gates/testgate.h>

g1::testgate tgate;

gxx::log::colored_stdout_target console_target;

int main() {
	g1::logger.link(console_target, gxx::log::level::trace);
	g1::link_gate(&tgate, 99);
	
	g1::address addr;
	addr.pushudp(G1_UDPGATE, "127.0.0.1", 5025);

	auto pptr = g1::send(addr, 9, "HelloWorld");
	g1::onestep_travel_only();

	gxx::println(pptr.ptr->status);
}