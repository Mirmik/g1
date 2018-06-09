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

	///Переместить пакет дальше по конвееру врат.
	void travell(g1::packet* pack); 
	void transport(g1::packet* pack); 

	///Вызывается на только что отправленный пакет. Башня или уничтожает его, или кеширует для контроля качества.
	void return_to_tower(g1::packet* pack, status sts);

	///Подключить врата к башне.
	inline void link_gate(g1::gateway* gate, uint8_t id) { 
		logger.info("gateway {} added", id);
		gateways.move_back(*gate);
		gate->id = id; 
	} 


	g1::gateway* find_target_gateway(const g1::packet* pack);
}

#endif