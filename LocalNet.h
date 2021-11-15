//
// Created by Herbert on 2021/11/13.
//

#ifndef YUXIANTECH_PIANOLESSON_LOCALNET_H
#define YUXIANTECH_PIANOLESSON_LOCALNET_H

#include <string>

namespace PianoLesson {

    class LocalNet {
    public:
        LocalNet();

        virtual ~LocalNet() = default;;

        const char *ipAddr() const;

        const char *broadcastAddr() const;

        const char *macAddr() const;

        int broadcastPort() const;

    private:
        bool generate();

        bool generateMac();

    private:
        std::string _ipAddr;
        std::string _broadcastAddr;
        std::string _macAddr;
    };

}

#endif //YUXIANTECH_PIANOLESSON_LOCALNET_H
