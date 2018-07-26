/**
@file tower.cpp
*/

#include <g1/tower.h>
#include <g1/indexes.h>
#include <gxx/util/hexascii.h>
#include <gxx/algorithm.h>
#include <gxx/util/string.h>
#include <gxx/syslock.h>
#include <gxx/print/stdprint.h>

gxx::dlist<g1::gateway, &g1::gateway::lnk> g1::gateways;
gxx::dlist<g1::packet, &g1::packet::lnk> g1::travelled;
gxx::dlist<g1::packet, &g1::packet::lnk> g1::incoming;
gxx::dlist<g1::packet, &g1::packet::lnk> g1::outters;
void(*g1::incoming_handler)(g1::packet* pack) = nullptr;
void(*g1::undelivered_handler)(g1::packet* pack) = nullptr;
void(*g1::traveling_handler)(g1::packet* pack) = nullptr;
void(*g1::transit_handler)(g1::packet* pack) = nullptr;


g1::gateway* g1::find_target_gateway(const g1::packet* pack) {
	uint8_t gidx = *pack->stageptr();
	for (auto& g : gateways) {
		if (g.id == gidx) return &g;
	}
	return nullptr;
}

void g1::release(g1::packet* pack) {
	gxx::syslock().lock();
	if (pack->released_by_tower) g1::utilize(pack);
	else pack->released_by_world = true;
	gxx::syslock().unlock();
}

void g1::tower_release(g1::packet* pack) {
	gxx::syslock().lock();
	dlist_del(&pack->lnk);
	if (pack->released_by_world) g1::utilize(pack);
	else pack->released_by_tower = true;
	gxx::syslock().unlock();
}

void utilize_from_outers(g1::packet* pack) {
	for (auto& el : g1::outters) {
		if (
			el.header.seqid == pack->header.seqid && 
			pack->header.alen == el.header.alen && 
			!memcmp(el.addrptr(), pack->addrptr(), pack->header.alen)
		) {
			g1::utilize(&el);
			return;
		}
	}
}

void qos_release_from_incoming(g1::packet* pack) {
	for (auto& el : g1::incoming) {
		if (el.header.seqid == pack->header.seqid && 
			pack->header.alen == el.header.alen && 
			!memcmp(el.addrptr(), pack->addrptr(), pack->header.alen)
		) {
			g1::tower_release(&el);
			return;
		}
	}
}

void add_to_incoming_list(g1::packet* pack) {
	pack->last_request_time = g1::millis();
	g1::incoming.move_back(*pack);
}

void add_to_outters_list(g1::packet* pack) {
	pack->last_request_time = g1::millis();
	g1::outters.move_back(*pack);
}

void g1::travel(g1::packet* pack) {
	gxx::syslock().lock();
	travelled.move_back(*pack);
	gxx::syslock().unlock();
}

void g1::travel_error(g1::packet* pack) {
	g1::utilize(pack);
}

void g1::do_travel(g1::packet* pack) {
	if (traveling_handler) traveling_handler(pack);
	if (pack->header.stg == pack->header.alen) {
		//Ветка доставленного пакета.
		g1::revert_address(pack);
		if (pack->header.ack) {
			switch(pack->header.type) {
				case G1_ACK_TYPE: utilize_from_outers(pack); break;
				case G1_ACK21_TYPE: utilize_from_outers(pack); send_ack2(pack); break;
				case G1_ACK22_TYPE: qos_release_from_incoming(pack); break;
				default: break;
			}
			g1::utilize(pack);
			return;
		}
		if (pack->ingate) g1::quality_notify(pack);	
		else g1::tower_release(pack);
		if (!pack->header.noexec && g1::incoming_handler) g1::incoming_handler(pack);
		else g1::release(pack);
		return;
	} 
	else {
		if (transit_handler) transit_handler(pack);
		//Ветка транзитного пакета. Логика поиска врат и пересылки.
		g1::gateway* gate = g1::find_target_gateway(pack);
		if (gate == nullptr) { 	
			g1::travel_error(pack);
		}
		else {
			//Здесь пакет штампуется временем отправки и пересылается во врата.
			//Врата должны после пересылки отправить его назад в башню
			//с помощью return_to_tower для контроля качества.
			gate->send(pack);
		}
	}
}

uint16_t __seqcounter = 0;
void g1::transport(g1::packet* pack) {
	pack->header.stg = 0;
	pack->header.ack = 0;
	gxx::syslock().lock();
	pack->header.seqid = __seqcounter++;
	gxx::syslock().unlock();
	g1::travel(pack);
}

//void g1::send(g1::address& addr, const char* data, size_t len, uint8_t type, g1::QoS qos, uint16_t ackquant) {
void g1::send(const void* addr, uint8_t asize, const char* data, uint16_t dsize, uint8_t type, g1::QoS qos, uint16_t ackquant) {
	g1::packet* pack = g1::create_packet(nullptr, asize, dsize);
	pack->header.type = type;
	pack->header.qos = qos;
	pack->header.ackquant = ackquant;
	//if (addr) 
	memcpy(pack->addrptr(), addr, asize);
	memcpy(pack->dataptr(), data, dsize);
	g1::transport(pack);
}

void g1::send(const void* addr, uint8_t asize, const gxx::iovec* vec, size_t veclen, uint8_t type, g1::QoS qos, uint16_t ackquant) {
	size_t dsize = 0;
	const gxx::iovec* it = vec;
	const gxx::iovec* const eit = vec + veclen;

	for (; it != eit; ++it) {
		dsize += it->size;
	}

	g1::packet* pack = g1::create_packet(nullptr, asize, dsize);
	pack->header.type = type;
	pack->header.qos = qos;
	pack->header.ackquant = ackquant;
	//if (addr) 
	memcpy(pack->addrptr(), addr, asize);

	it = vec;
	char* dst = pack->dataptr(); 
	for (; it != eit; ++it) {
		memcpy(dst, it->data, it->size);
		dst += it->size;
	}
	
	g1::transport(pack);
}


void g1::quality_notify(g1::packet* pack) {
	if (pack->header.qos == g1::TargetACK || pack->header.qos == g1::BinaryACK) {
		g1::send_ack(pack);
	}
	if (pack->header.qos == g1::BinaryACK) add_to_incoming_list(pack);
	else tower_release(pack);
}

void g1::return_to_tower(g1::packet* pack, g1::status sts) {
	gxx::system_lock();
	if (pack->ingate != nullptr) {
		//Пакет был отправлен, и он нездешний. Уничтожить.
		g1::utilize(pack);
	} else {
		//Пакет здешний.
		if (sts != g1::status::Sended || pack->header.qos == WithoutACK) 
			g1::utilize(pack);
		else add_to_outters_list(pack);
	}
	gxx::system_unlock();
}

void g1::print(g1::packet* pack) {
	g1:print_to(*gxx::standart_output, pack);
}

void g1::println(g1::packet* pack) {
	g1:print_to(*gxx::standart_output, pack);
	gxx::print_to(*gxx::standart_output, "\n");
}

void g1::print_to(gxx::io::ostream& out, g1::packet* pack) {
	gxx::fprint_to(out, "("
		"qos:{}, "
		"alen:{}, "
		"type:{}, "
		"addr:{}, "
		"stg:{}, "
		"data:{}, "
		"released:{}"
		")", 
		pack->header.qos, 
		pack->header.alen, 
		(uint8_t)pack->header.type, 
		gxx::hexascii_encode((const uint8_t*)pack->addrptr(), pack->header.alen), 
		pack->header.stg, 
		gxx::dstring(pack->dataptr(), pack->datasize()), 
		pack->flags
	);
}

void g1::revert_address(g1::packet* pack) {
	auto first = pack->addrptr();
	auto last = pack->addrptr() + pack->header.alen;
	while ((first != last) && (first != --last)) {
        char tmp = *last;
        *last = *first;
        *first++ = tmp;
    }
}

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

void g1::send_ack2(g1::packet* pack) {
	auto ack = g1::create_packet(nullptr, pack->header.alen, 0);
	ack->header.type = G1_ACK22_TYPE;
	ack->header.ack = 1;
	ack->header.qos = g1::QoS::WithoutACK;
	ack->header.seqid = pack->header.seqid;
	memcpy(ack->addrptr(), pack->addrptr(), pack->header.alen);
	g1::travel(ack);
}

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

	uint16_t curtime = g1::millis();
	gxx::for_each_safe(g1::outters.begin(), g1::outters.end(), [&](g1::packet& pack) {
		if (curtime - pack.last_request_time > pack.header.ackquant) {
			dlist_del(&pack.lnk);
			if (++pack.ackcount == 5) {
				if (g1::undelivered_handler) g1::undelivered_handler(&pack);
				else g1::utilize(&pack);
			} else {
				g1::travel(&pack);
			}		
		}
	});

	gxx::for_each_safe(g1::incoming.begin(), g1::incoming.end(), [&](g1::packet& pack) {
		if (curtime - pack.last_request_time > pack.header.ackquant) {
			dlist_del(&pack.lnk);
			if (++pack.ackcount == 5) g1::utilize(&pack);
			else g1::send_ack(&pack);	
		}
	});
	lock.unlock();
}

void g1::spin() {
	while(1) {
		for (auto& gate : gateways) gate.nonblock_onestep();
		g1::onestep();
	}
}