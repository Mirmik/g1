#ifndef G1_GATEWAY_H
#define G1_GATEWAY_H

namespace g1 {
	class travelled_package;

	/**
		Абстрактный класс врат. Врата отвечают за пересылку пакетов между мирами.
	*/
	struct gateway {
		dlist_head lnk; ///< встроенное поле списка.
		uint16_t id; ///< номер врат.
		
		///Очередь пакетов, ожидающих отправки.
		gxx::dlist<packet, &packet::lnk> packq;

		
		/** 
			@brief Отправить пакет в целевой мир, согласно адресу стадии.
			@details Убить зверя.
			@param pack Пересылаемый пакет
			@return Статус ошибки.
		*/
		virtual int send(const packet& pack) = 0;

		/**
			@brief Обработка пакета, пришедшего из другого мира.
			@details Здесь происходит реверт адреса и переправка в башню.
		*/
		int receive_handler(void* buf, size_t sz);
	};

}

#endif