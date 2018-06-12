/**
@file tower.cpp
*/

#include <g1/tower.h>
#include <g1/packet.h>
#include <g1/gateway.h>
#include <g1/indexes.h>

#include <gxx/print.h>
#include <gxx/util/hexascii.h>
#include <gxx/trace.h>

#include <gxx/algorithm.h>
#include <gxx/atomic_section.h>

using namespace gxx::argument_literal;

gxx::dlist<g1::gateway, &g1::gateway::lnk> g1::gateways;
gxx::dlist<g1::packet, &g1::packet::lnk> g1::incoming;
gxx::dlist<g1::packet, &g1::packet::lnk> g1::outters;
void(*g1::incoming_handler)(g1::packet* pack) = nullptr;
void(*g1::undelivered_handler)(g1::packet* pack) = nullptr;
gxx::log::logger g1::logger("g1");
static gxx::atomic_section atomic;

g1::gateway* g1::find_target_gateway(const g1::packet* pack) {
	uint8_t gidx = pack->gateway_index();
	for (auto& g : gateways) {
		if (g.id == gidx) return &g;
	}
	g1::logger.warn("WrongGate: {0}", gidx);
	return nullptr;
}

void g1::release(g1::packet* pack) {
	g1::logger.debug("released by world");
	pack->released_by_world = true;
	g1::release_if_need(pack);
}

void g1::qos_release(g1::packet* pack) {
	g1::logger.debug("released by tower");
	pack->released_by_tower = true;
	g1::release_if_need(pack);
}

void utilize_from_outers(g1::packet* pack) {
	g1::logger.debug("utilize_from_list");
	for (auto& el : g1::outters) {
		if (el.block->seqid == pack->block->seqid && el.addrsect() == pack->addrsect()) {
			g1::utilize(&el);
			return;
		}
	}
}

void qos_release_from_incoming(g1::packet* pack) {
	for (auto& el : g1::incoming) {
		if (el.block->seqid == pack->block->seqid && el.addrsect() == pack->addrsect()) {
			g1::qos_release(&el);
			return;
		}
	}
}

void add_to_incoming_list(g1::packet* pack) {
	atomic.lock();
	g1::incoming.move_back(*pack);
	atomic.unlock();
}

void add_to_outters_list(g1::packet* pack) {
	atomic.lock();
	g1::outters.move_back(*pack);
	atomic.unlock();	
}

void g1::travell(g1::packet* pack) {
	g1::print(pack);
	if (pack->block->stg == pack->block->alen) {
		g1::revert_address(pack);
		if (pack->block->ack) {
			switch(pack->block->type) {
				case G1_ACK_TYPE: utilize_from_outers(pack); break;
				case G1_ACK21_TYPE: utilize_from_outers(pack); send_ack2(pack); break;
				case G1_ACK22_TYPE: qos_release_from_incoming(pack); break;
				default: break;
			}
			g1::utilize(pack);
			return;
		}
		if (pack->ingate) g1::quality_notify(pack);			
		if (!pack->block->noexec && g1::incoming_handler) g1::incoming_handler(pack);
		else g1::release(pack);
		return;
	} else {
		//Ветка транзитного пакета. Логика поиска врат и пересылки.
		g1::gateway* gate = g1::find_target_gateway(pack);
		if (gate == nullptr) g1::utilize(pack);
		else {
			//Здесь пакет штампуется временем отправки и пересылается во врата.
			//Врата должны после пересылки отправить его назад в башню
			//с помощью return_to_tower для контроля качества.
			pack->last_request_time = g1::millis();
			gate->send(pack);
		}
	}
}

uint16_t __seqcounter = 0;
void g1::transport(g1::packet* pack) {
	pack->block->stg = 0;
	pack->block->ack = 0;
	pack->block->seqid = __seqcounter++;
	g1::travell(pack);
}

void g1::quality_notify(g1::packet* pack) {
	if (pack->block->qos == g1::TargetACK || pack->block->qos == g1::BinaryACK) {
		g1::send_ack(pack);
	}
	if (pack->block->qos == g1::BinaryACK) add_to_incoming_list(pack);
	else qos_release(pack);
}

void g1::return_to_tower(g1::packet* pack, g1::status sts) {
	if (pack->ingate != nullptr || sts != g1::status::Sended || pack->block->qos == WithoutACK) 
		g1::utilize(pack);
	else add_to_outters_list(pack);
}

void g1::print(g1::packet* pack) {
	gxx::fprintln("(type:{}, addr:{}, stg:{}, data:{}, released:{})", (uint8_t)pack->block->type, gxx::hexascii_encode((const uint8_t*)pack->addrptr(), pack->block->alen), pack->block->stg, pack->datasect(), pack->flags);
}

void g1::release_if_need(g1::packet* pack) {
	if (pack->released_by_tower && pack->released_by_world) utilize(pack);
}

void g1::revert_address(g1::packet* pack) {
	gxx::buffer addr = pack->addrsect();
	auto first = addr.begin();
	auto last = addr.end();
	while ((first != last) && (first != --last)) {
        std::iter_swap(first++, last);
    }
}

void g1::send_ack(g1::packet* pack) {
	auto block = g1::create_block(pack->block->alen, 0);
	auto ack = g1::create_packet(nullptr, block);
	ack->set_type(pack->block->qos == g1::QoS::BinaryACK ? G1_ACK21_TYPE : G1_ACK_TYPE);
	ack->block->ack = 1;
	ack->block->qos = g1::QoS::WithoutACK;
	ack->block->seqid = pack->block->seqid;
	memcpy(ack->addrptr(), pack->addrptr(), pack->block->alen);
	g1::travell(ack);
}

void g1::send_ack2(g1::packet* pack) {
	auto block = g1::create_block(pack->block->alen, 0);
	auto ack = g1::create_packet(nullptr, block);
	ack->set_type(G1_ACK22_TYPE);
	ack->block->ack = 1;
	ack->block->qos = g1::QoS::WithoutACK;
	ack->block->seqid = pack->block->seqid;
	memcpy(ack->addrptr(), pack->addrptr(), pack->block->alen);
	g1::travell(ack);
}

/**
	@todo Переделать очередь пакетов, выстроив их в порядке работы таймеров. Это ускорит quality_work_execute
*/
void g1::quality_work_execute() {
	uint16_t curtime = g1::millis();
	
	gxx::for_each_safe(g1::outters.begin(), g1::outters.end(), [&](g1::packet& pack) {
		if (curtime - pack.last_request_time > pack.block->ackquant) {
			g1::logger.debug("ack quant in outters, {}", pack.ackcount);
			dlist_del(&pack.lnk);
			if (++pack.ackcount == 5) {
				g1::logger.debug("undelivered packet in outters");
				if (g1::undelivered_handler) g1::undelivered_handler(&pack);
				else g1::utilize(&pack);
			} else {
				g1::travell(&pack);
			}		
		}
	});

	gxx::for_each_safe(g1::incoming.begin(), g1::incoming.end(), [&](g1::packet& pack) {
		if (curtime - pack.last_request_time > pack.block->ackquant) {
			g1::logger.debug("ack quant in incomming, {}", pack.ackcount);
			dlist_del(&pack.lnk);
			if (++pack.ackcount == 5) {
				g1::logger.debug("undelivered ack in incoming");
				g1::utilize(&pack);
			} else {
				g1::send_ack(&pack);
			}		
		}
	});
}

#include <gxx/inet/dgramm.h>
void g1::pushudp(std::string& addr, const char* ip, uint16_t port) {
	addr.resize(addr.size() + 6);
	*(uint32_t*)(addr.data() + addr.size() - 6) = inet_addr(ip);
	*(uint32_t*)(addr.data() + addr.size() - 2) = htons(port);
}

void g1::pushgate(std::string& addr, uint8_t c) {
	addr.push_back(c);
}