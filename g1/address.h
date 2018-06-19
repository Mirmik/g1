#ifndef G1_ADDRESS_H
#define G1_ADDRESS_H

#include <string>
#include <gxx/inet/dgramm.h>

namespace g1 {
	struct address {
		std::string str;

		address(uint8_t u8) { str.push_back(u8); }
		address(){};

		void pushudp(uint8_t gate, const char* ip, uint16_t port) {
			str.resize(str.size() + 6);
			*(uint32_t*)(str.data() + str.size() - 6) = inet_addr(ip);
			*(uint32_t*)(str.data() + str.size() - 2) = htons(port);
		}
	};
}

#endif