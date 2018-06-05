/**
@file 
@brief G1 main file
*/

#ifndef G1_H
#define G1_H

#include <cstdint>

namespace g1 {
	/// Качество обслуживания.
	enum QoS : uint8_t {
		One, ///< one
		Two, ///< two
		Three ///< three
	};

	///Структура заголовок пакета. 
	struct package_header {
		uint8_t type; ///< Тип 
		uint16_t flen;
		uint16_t seqid;
		uint8_t alen;
		uint8_t stg;
		QoS qos; 
	};

	///Структура-описатель блока. Создается поверх пакета для упрощения работы с ним.
	struct package_block {
		package_header* bptr; ///< Указатель на заголовок реферируемого блока
	};

}

#endif