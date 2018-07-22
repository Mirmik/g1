/**
@file 
@brief Всё, что касается работы с пакетом.
*/

#ifndef G1_PACKAGE_H
#define G1_PACKAGE_H

#include <cstdint>
#include <gxx/buffer.h>
#include <gxx/datastruct/dlist_head.h>

#define G1_PACKED __attribute__((packed))

namespace g1 {
	struct gateway;

	/// Качество обслуживания.
	enum QoS : uint8_t {
		WithoutACK = 0, ///< one
		TargetACK = 1, ///< two
		BinaryACK = 2 ///< three
	};

	/**
		@brief Структура заголовок пакета. 
		@details Заголовок пакета располагается в первых байтах пакета.
		за заголовком следует поле адреса переменной длины, а за ним данные.
	*/
	struct packet_header {
		union {
			uint8_t pflag; ///< Флаги пакета
			struct {
				uint8_t ack : 1; ///< Идентифицирует ack пакеты. Доп.инф. передается в типе.
				uint8_t vaddr : 1; ///< Поле указатель виртуального адреса @todo
				uint8_t noexec : 1; ///< Флаг предотвращает исполнение пакета. Используется для запросов существования
				uint8_t type : 5; ///< Доп. инф. зависит от ситуации.
			};
		};
		uint16_t flen; ///< Полная длина пакета
		uint8_t alen; ///< Длина поля адреса.
		uint8_t stg; ///< Поля стадии. Используется для того, чтобы цепочка врат знала, какую часть адреса обрабатывать.
		uint16_t ackquant; ///< Таймаут для пересылки пакета.
		uint16_t seqid; ///< Порядковый номер пакета. Присваивается отправителем.
		QoS qos; ///< Поле качества обслуживания.
	} G1_PACKED;

	struct packet {
		dlist_head lnk; ///< Для подключения в список.
		g1::gateway* ingate; ///< gate, которым пакет прибыл в систему.
		uint16_t last_request_time; ///< @todo
		uint8_t ackcount; ///< @todo
		uint8_t status;

		union {
			uint8_t flags; ///< Местные флаги
			struct {
				uint8_t released_by_world : 1;
				uint8_t released_by_tower : 1;
			};
		};

		packet_header header;

		uint8_t* addrptr() const { return (uint8_t*)(&header + 1); }
		char* dataptr() const { return (char*)(&header + 1) + header.alen; }
		uint8_t* stageptr() const { return (uint8_t*)(&header + 1) + header.stg; }
		char* endptr() const { return (char*)(&header) + header.flen; }

		size_t addrsize() { return header.alen; }
		size_t blocksize() { return header.flen; }
		size_t datasize() { return header.flen - header.alen - sizeof(packet_header); }

		void pushaddr(uint8_t u8) { addrptr()[header.stg++] = u8; }
	
		void revert_stage(void* addr1, uint8_t size1, void* addr2, uint8_t size2, uint8_t gateindex);
		void revert_stage(void* addr, uint8_t size, uint8_t gateindex);
		void revert_stage(uint8_t gateindex);
	} G1_PACKED;

	packet* allocate_packet(size_t adlen); 
	packet* create_packet(gateway* ingate, size_t addrsize, size_t datasize); 
	void packet_initialization(g1::packet* pack, gateway* ingate); 
	void utilize_packet(packet* pack);

	void utilize(packet* pack);
}

#endif