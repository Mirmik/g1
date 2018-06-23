/**
@file
@brief tower file
*/

#ifndef G1_TOWER_H
#define G1_TOWER_H

#include <gxx/container/dlist.h>
#include <g1/gateway.h>

#include <gxx/log/logger2.h>

namespace g1 {
	enum class status : uint8_t {
		Sended,
		WrongAddress,
	};

	extern gxx::log::logger logger;

	///Список врат.
	extern gxx::dlist<g1::gateway, &g1::gateway::lnk> gateways;
	extern gxx::dlist<g1::packet, &g1::packet::lnk> incoming;
	extern gxx::dlist<g1::packet, &g1::packet::lnk> outters;
	extern gxx::dlist<g1::packet, &g1::packet::lnk> travelled;

	///Переместить пакет дальше по конвееру врат.
	void travel_error(g1::packet* pack); 
	void travel(g1::packet* pack); 
	void do_travel(g1::packet* pack); 
	
	void transport(g1::packet* pack); 
	void send(const char* addr, uint8_t asize, const char* data, uint16_t dsize, uint8_t type = 0, g1::QoS qos = (g1::QoS)0, uint16_t ackquant = 20);
	
	///Вызывается на только что отправленный пакет. Башня или уничтожает его, или кеширует для контроля качества.
	void return_to_tower(g1::packet* pack, status sts);

	///Вызывается на принятый пакет. Выполняет кеширование (если надо) и отправку ACK пакетов.
	void quality_notify(g1::packet* pack);

	///Подключить врата к башне.
	inline void link_gate(g1::gateway* gate, uint8_t id) { 
		//logger.debug("gateway {} added", id);
		gateways.move_back(*gate);
		gate->id = id; 
	} 


	g1::gateway* find_target_gateway(const g1::packet* pack);


	void release(g1::packet* pack);
	void tower_release(g1::packet* pack);
	void print(g1::packet* pack);

	void revert_address(g1::packet* pack);

	void send_ack(g1::packet* pack);
	void send_ack2(g1::packet* pack);

	extern void (*incoming_handler)(g1::packet* pack);

	/// Обработчик недоставленного пакета. Определяется локальным софтом.
	/// Освобождение должно производиться функцией tower_release.
	extern void(*undelivered_handler)(g1::packet* pack);

	uint16_t millis();

	void onestep();
	void onestep_travel_only();
	void spin();
}

#endif