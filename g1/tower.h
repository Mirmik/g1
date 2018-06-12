/**
@file
@brief tower file
*/

#ifndef G1_TOWER_H
#define G1_TOWER_H

#include <gxx/container/dlist.h>
#include <gxx/log/logger2.h>

#include <g1/gateway.h>
#include <vector>

namespace g1 {
	extern gxx::log::logger logger;

	class gateway;

	enum class status : uint8_t {
		Sended,
		WrongGate,
		WrongAddress,
	};

	///Список врат.
	extern gxx::dlist<g1::gateway, &g1::gateway::lnk> gateways;
	extern gxx::dlist<g1::packet, &g1::packet::lnk> incoming;
	extern gxx::dlist<g1::packet, &g1::packet::lnk> outters;

	///Переместить пакет дальше по конвееру врат.
	void travell(g1::packet* pack); 
	void transport(g1::packet* pack); 

	///Вызывается на только что отправленный пакет. Башня или уничтожает его, или кеширует для контроля качества.
	void return_to_tower(g1::packet* pack, status sts);

	///Вызывается на принятый пакет. Выполняет кеширование (если надо) и отправку ACK пакетов.
	void quality_notify(g1::packet* pack);

	///Подключить врата к башне.
	inline void link_gate(g1::gateway* gate, uint8_t id) { 
		logger.info("gateway {} added", id);
		gateways.move_back(*gate);
		gate->id = id; 
	} 


	g1::gateway* find_target_gateway(const g1::packet* pack);


	void release(g1::packet* pack);
	void qos_release(g1::packet* pack);
	void print(g1::packet* pack);

	void revert_address(g1::packet* pack);

	void send_ack(g1::packet* pack);
	void send_ack2(g1::packet* pack);

	void release_if_need(g1::packet* pack);

	extern void (*incoming_handler)(g1::packet* pack);

	/** @brief Проведение работ по обеспечению качества обслуживания.
		@details может вызывать g1::undelivered_handler
	*/
	void quality_work_execute();

	/// Обработчик недоставленного пакета. Определяется локальным софтом.
	extern void(*undelivered_handler)(g1::packet* pack);

	uint16_t millis();
}

#endif