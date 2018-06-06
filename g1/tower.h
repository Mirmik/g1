/**
@file
@brief tower file
*/

#ifndef G1_TOWER_H
#define G1_TOWER_H

#include <gxx/container/dlist.h>
#include <g1/gateway.h>

namespace g1 {
	class gateway;

	extern gxx::dlist<g1::gateway, &g1::gateway::lnk> gateways; ///Список индексированных врат

	///Переместить пакет дальше по конвееру врат.
	void transport(g1::package pack); 

	///Подключить врата к башне.
	inline void registry_gate(g1::gateway& gate) { gateways.move_back(gate); } 

}

#endif