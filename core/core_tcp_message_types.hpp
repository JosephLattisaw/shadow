#ifndef CORE_TCP_MESSAGE_TYPES_HPP
#define CORE_TCP_MESSAGE_TYPES_HPP

#include <cstdint>

namespace core {
struct header {
    enum MESSAGE_TYPE {
        KEEP_ALIVE = 0,
    };

    std::uint32_t message_type;
    std::uint32_t packet_size = 0; //bytes
};


}

#endif // CORE_TCP_MESSAGE_TYPES_HPP
