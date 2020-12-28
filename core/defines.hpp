#ifndef DEFINES_HPP
#define DEFINES_HPP

#include <QString>

namespace shadow {
    const QString APPLICATION_DESC = "STS Shadow: Background application which acts as controller for STS-jwst simulator.";
    const QString APPLICATION_NAME = "STS Shadow";
    const QString VERSION_NUMBER = "1.0.0";

    enum APP_STATUS {
        NOT_RUNNING = 0,
        RUNNING = 1,
        CRASH = 2,
        FAILED_TO_START = 3,
    };
}

#endif // DEFINES_HPP
