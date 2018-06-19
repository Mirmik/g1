/**
@file
@brief tower file
*/

#ifndef G1_TOWER_H
#define G1_TOWER_H

#include <g1/address.h>

#include <gxx/container/dlist.h>
#include <gxx/log/logger2.h>

#include <g1/gateway.h>
#include <vector>

namespace g1 {
	extern gxx::log::logger logger;

	enum class status : uint8_t {
		Sended,
//		WrongGate,
		WrongAddress,
	};

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
	g1::packptr send(g1::packet* pack); 
	g1::packptr send(g1::address& addr, uint8_t type, const char* str); 
	g1::packptr send(g1::address& addr, uint8_t type, const void* data, size_t size);
	g1::packptr send(g1::address& addr, uint8_t type, const std::string& str);
	
	///Вызывается на только что отправленный пакет. Башня или уничтожает его, или кеширует для контроля качества.
	void return_to_tower(g1::packet* pack, status sts);

/*	///Вызывается на принятый пакет. Выполняет кеширование (если надо) и отправку ACK пакетов.
	void quality_notify(g1::packet* pack);
*/
	///Подключить врата к башне.
	inline void link_gate(g1::gateway* gate, uint8_t id) { 
		logger.info("gateway {} added", id);
		gateways.move_back(*gate);
		gate->id = id; 
	} 


	g1::gateway* find_target_gateway(const g1::packet* pack);


	void release(g1::packet* pack);
	void tower_release(g1::packet* pack);
	/*void print(g1::packet* pack);

	void revert_address(g1::packet* pack);
*/
	void send_ack(g1::packet* pack);
	void send_ack2(g1::packet* pack);
/*
	void release_if_need(g1::packet* pack);
*/

	extern void (*incoming_handler)(g1::packet* pack);

	/// Обработчик недоставленного пакета. Определяется локальным софтом.
	/// Освобождение должно производиться функцией tower_release.
	extern void(*undelivered_handler)(g1::packet* pack);

	/** @brief Проведение работ по обеспечению качества обслуживания.
		@details может вызывать g1::undelivered_handler
	*/

	uint16_t millis();
/*
	void pushudp(std::string& addr, const char* ip, uint16_t port);
	void pushgate(std::string& addr, uint8_t gate);
*/


	void onestep();
	void onestep_travel_only();
	void spin();
}

#endif