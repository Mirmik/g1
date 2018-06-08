/**
@file 
@brief Всё, что касается работы с пакетом.
*/

#ifndef G1_PACKAGE_H
#define G1_PACKAGE_H

#include <cstdint>
#include <gxx/refcountered.h>

namespace g1 {
	/// Качество обслуживания.
	enum QoS : uint8_t {
		One, ///< one
		Two, ///< two
		Three ///< three
	};

	/**
		@brief Структура заголовок пакета. 
		@details Заголовок пакета располагается в первых байтах пакета.
		за заголовком следует поле адреса переменной длины, а за ним данные.
	*/
	struct packet_header {
		uint8_t type; ///< Тип.
		uint16_t flen; ///< Полная длина пакета
		uint16_t seqid; ///< Порядковый номер пакета. Присваивается отправителем.
		uint8_t alen; ///< Длина поля адреса.
		uint8_t stg; ///< Поля стадии. Используется для того, чтобы цепочка врат знала, какую часть адреса обрабатывать.
		QoS qos; ///< Поле качества обслуживания.
	};

	///Структура-описатель блока. Создается поверх пакета для упрощения работы с ним.
	struct packet {
		dlist_head lnk; /// < Для подключения в список.
		g1::gateway* ingate; /// < gate, которым пакет прибыл в систему.

		//dlist_head tlnk;
		packet_header* bptr; ///< Указатель на заголовок реферируемого блока

		///Отметить в пакете прохождение врат.
		void revert_stage(uint8_t size, void* addr);

		uint8_t gateway_index();
		gxx::buffer gateway_address(uint8_t asz);
	};

	packet_header* allocate_block(size_t asz, size_t bsz);
	void utilize_block(packet_header* block);

	packet* allocate_packet(); 
	void utilize_packet(packet* pack);
}

#endif