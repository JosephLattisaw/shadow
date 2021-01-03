#ifndef CORE_TCP_MESSAGE_TYPES_HPP
#define CORE_TCP_MESSAGE_TYPES_HPP

#include <cstdint>
#include <QByteArray>
#include <QVector>

namespace core {
struct header {
    enum MESSAGE_TYPE {
        KEEP_ALIVE = 0,
        START = 1,
        STOP = 2,
        STATUS = 3,
    };

    std::uint32_t message_type;
    std::uint32_t packet_size = 0; //bytes
};

struct status_message {
    std::uint32_t app_name_size = 0;
    std::uint32_t cla_size = 0;
    std::uint32_t status_size = 0;

    std::uint32_t *app_name_sizes;
    QByteArray *app_name_data;

    std::uint32_t *cla_sizes;
    QByteArray *cla_data;

    std::uint32_t *statuses;
};
}

#endif // CORE_TCP_MESSAGE_TYPES_HPP
