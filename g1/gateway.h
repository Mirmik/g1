#ifndef G1_GATEWAY_H
#define G1_GATEWAY_H

#include <g1/packet.h>
#include <gxx/container/dlist.h>

namespace g1 {
	/**
		Абстрактный класс врат. Врата отвечают за пересылку пакетов между мирами.
	*/
	struct gateway {
		dlist_head lnk; ///< встроенное поле списка.
		uint16_t id; ///< номер врат.
		
		gateway() {
			dlist_init(&lnk);
		}

		///Очередь пакетов, ожидающих отправки.
		gxx::dlist<packet, &packet::lnk> packq;

		
		/** 
			@brief Отправить пакет в целевой мир, согласно адресу стадии.
			@details Убить зверя.
			@param pack Пересылаемый пакет
			@return Статус ошибки.
		*/
		virtual void send(packet* pack) = 0;

		/**
			@brief Обработка пакета, пришедшего из другого мира.
			@details Здесь происходит реверт адреса и переправка в башню.
		*/
		int receive_handler(void* buf, size_t sz);
	};

}

#endif