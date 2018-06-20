/**
@file tower.cpp
*/

#include <g1/tower.h>
/*#include <g1/packet.h>
#include <g1/gateway.h>*/
#include <g1/indexes.h>/*
*/
#include <gxx/print.h>
#include <gxx/util/hexascii.h>
/*#include <gxx/trace.h>
*/
#include <gxx/algorithm.h>

#include <gxx/syslock.h>
/*
using namespace gxx::argument_literal;
*/
gxx::dlist<g1::gateway, &g1::gateway::lnk> g1::gateways;
gxx::dlist<g1::packet, &g1::packet::lnk> g1::travelled;
gxx::dlist<g1::packet, &g1::packet::lnk> g1::incoming;
gxx::dlist<g1::packet, &g1::packet::lnk> g1::outters;
void(*g1::incoming_handler)(g1::packet* pack) = nullptr;
void(*g1::undelivered_handler)(g1::packet* pack) = nullptr;

gxx::log::logger g1::logger("g1");

g1::gateway* g1::find_target_gateway(const g1::packet* pack) {
	uint8_t gidx = *pack->stageptr();
	for (auto& g : gateways) {
		if (g.id == gidx) return &g;
	}
	return nullptr;
}

void g1::release(g1::packet* pack) {
	g1::logger.trace("release");
	gxx::syslock().lock();
	if (pack->released_by_tower) g1::utilize(pack);
	else pack->released_by_world = true;
	gxx::syslock().unlock();
}

void g1::tower_release(g1::packet* pack) {
	g1::logger.trace("tower release");
	gxx::syslock().lock();
	dlist_del(&pack->lnk);
	if (pack->released_by_world) g1::utilize(pack);
	else pack->released_by_tower = true;
	gxx::syslock().unlock();
}
/*
void utilize_from_outers(g1::packet* pack) {
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
*/
void add_to_incoming_list(g1::packet* pack) {
	g1::incoming.move_back(*pack);
}

void add_to_outters_list(g1::packet* pack) {
	g1::outters.move_back(*pack);
}

void g1::travel(g1::packet* pack) {
	gxx::syslock().lock();
	travelled.move_back(*pack);
	gxx::syslock().unlock();
}

void g1::travel_error(g1::packet* pack) {
	if (pack->ingate != nullptr) g1::utilize(pack);
	else g1::tower_release(pack);
}

void g1::do_travel(g1::packet* pack) {
	g1::print(pack);
	if (pack->header.stg == pack->header.alen) {
		/*g1::revert_address(pack);
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
		return;*/
	} else {
		//Ветка транзитного пакета. Логика поиска врат и пересылки.
		g1::gateway* gate = g1::find_target_gateway(pack);
		if (gate == nullptr) { 	
			g1::logger.warn("WrongGate: {0}", *pack->stageptr());
			g1::travel_error(pack);
		}
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
	pack->header.stg = 0;
	pack->header.ack = 0;
	pack->header.seqid = __seqcounter++;
	g1::travel(pack);
}

g1::packptr g1::send(g1::packet* pack) {
	g1::transport(pack);
	return g1::packptr { pack }; 
}

g1::packptr g1::send(g1::address& addr, uint8_t type, const char* data, size_t len) {
	g1::packet* pack = create_packet(nullptr, addr.str.size(), len);
	pack->header.type =type;
	memcpy(pack->addrptr(), addr.str.data(), addr.str.size());
	memcpy(pack->dataptr(), data, len);
	g1::transport(pack);
	return g1::packptr { pack }; 
}

g1::packptr g1::send(g1::address& addr, uint8_t type, const char* str) {
	return g1::send(addr, type, str, strlen(str));
}

g1::packptr g1::send(g1::address& addr, uint8_t type, const std::string& str) {
	return g1::send(addr, type, str.data(), str.size());
}
/*
void g1::quality_notify(g1::packet* pack) {
	if (pack->block->qos == g1::TargetACK || pack->block->qos == g1::BinaryACK) {
		g1::send_ack(pack);
	}
	if (pack->block->qos == g1::BinaryACK) add_to_incoming_list(pack);
	else qos_release(pack);
}
*/
void g1::return_to_tower(g1::packet* pack, g1::status sts) {
	if (pack->ingate != nullptr) {
		//Пакет был отправлен, и он нездешний. Уничтожить.
		g1::utilize(pack);
	} else {
		//Пакет здешний.
		if (sts != g1::status::Sended || pack->header.qos == WithoutACK) 
			g1::tower_release(pack);
		else add_to_outters_list(pack);
	}

	//if (pack->ingate != nullptr || sts != g1::status::Sended || pack->block->qos == WithoutACK) 
	//	g1::utilize(pack);
	//else add_to_outters_list(pack);
}

void g1::print(g1::packet* pack) {
	g1::logger.info("(type:{}, addr:{}, stg:{}, data:{}, released:{})", (uint8_t)pack->header.type, gxx::hexascii_encode((const uint8_t*)pack->addrptr(), pack->header.alen), pack->header.stg, gxx::buffer(pack->dataptr(), pack->datasize()), pack->flags);
}
/*
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
*/
void g1::send_ack(g1::packet* pack) {
	auto ack = g1::create_packet(nullptr, pack->header.alen, 0);
	ack->header.type = pack->header.qos == g1::QoS::BinaryACK ? G1_ACK21_TYPE : G1_ACK_TYPE;
	ack->header.ack = 1;
	ack->header.qos = g1::QoS::WithoutACK;
	ack->header.seqid = pack->header.seqid;
	memcpy(ack->addrptr(), pack->addrptr(), pack->header.alen);
	ack->released_by_world = true;
	g1::travel(ack);
}
/*
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
*/
void g1::onestep_travel_only() {
	gxx::syslock lock;
	while(1) {
		lock.lock();
		bool empty = g1::travelled.empty();
		if (empty) break;
		g1::packet* pack = &*g1::travelled.begin();
		g1::travelled.unbind(*pack);
		lock.unlock();
		g1::do_travel(pack);
	} 
	lock.unlock();
}

/**
	@todo Переделать очередь пакетов, выстроив их в порядке работы таймеров. 
*/
void g1::onestep() {
	gxx::syslock lock;
	while(1) {
		lock.lock();
		bool empty = g1::travelled.empty();
		if (empty) break;
		g1::packet* pack = &*g1::travelled.begin();
		g1::travelled.unbind(*pack);
		lock.unlock();
		g1::do_travel(pack);
	} 
	lock.unlock();

	uint16_t curtime = g1::millis();
	
	gxx::for_each_safe(g1::outters.begin(), g1::outters.end(), [&](g1::packet& pack) {
		if (curtime - pack.last_request_time > pack.header.ackquant) {
			g1::logger.debug("ack quant in outters, {}", pack.ackcount);
			dlist_del(&pack.lnk);
			if (++pack.ackcount == 5) {
				g1::logger.debug("undelivered packet in outters");
				if (g1::undelivered_handler) g1::undelivered_handler(&pack);
				else g1::utilize(&pack);
			} else {
				g1::travel(&pack);
			}		
		}
	});

	gxx::for_each_safe(g1::incoming.begin(), g1::incoming.end(), [&](g1::packet& pack) {
		if (curtime - pack.last_request_time > pack.header.ackquant) {
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


void g1::spin() {
	while(1) g1::onestep();
}
/*

#include <gxx/inet/dgramm.h>
void g1::pushudp(std::string& addr, const char* ip, uint16_t port) {
	addr.resize(addr.size() + 6);
	*(uint32_t*)(addr.data() + addr.size() - 6) = inet_addr(ip);
	*(uint32_t*)(addr.data() + addr.size() - 2) = htons(port);
}

void g1::pushgate(std::string& addr, uint8_t c) {
	addr.push_back(c);
}*/