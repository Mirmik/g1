/**
@file
@brief tower file
*/

#ifndef G1_TOWER_H
#define G1_TOWER_H

#include <gxx/container/dlist.h>
#include <g1/gateway.h>
#include <vector>

namespace g1 {
	class gateway;

	enum class status : uint8_t {
		Sended,
		WrongGate,
		WrongAddress,
	};

	///Список врат.
	extern gxx::dlist<g1::gateway, &g1::gateway::lnk> gateways;

	///Переместить пакет дальше по конвееру врат.
	void transport(const g1::packet& pack); 

	///Вызывается на только что отправленный пакет. Башня или уничтожает его, или кеширует для контроля качества.
	void return_to_tower(const g1::packet& pack, uint8_t status);

	///Подключить врата к башне.
	inline void link_gate(g1::gateway& gate) { gateways.move_back(gate); } 
}

#endif