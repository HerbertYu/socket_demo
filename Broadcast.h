//
// Created by Herbert on 2021/11/13.
//

#ifndef YUXIANTECH_PIANOLESSON_BROADCAST_H
#define YUXIANTECH_PIANOLESSON_BROADCAST_H

#include <thread>
#include <functional>

namespace PianoLesson {

    class Message;

    class Broadcast {
    public:
        Broadcast(const char *ip, int port);

        ~Broadcast();

        void setCallback(std::function<void(const char *ip, const Message *msg)> callback);

        bool send(const char *msg) const;

        bool send(const Message *message) const;

    private:
        bool init(const char *ip, int port);

        void listen() const;

    private:
        int _sendSockFd{};
        int _recvSockFd{};
        bool _started{false};
        std::thread _listen;
        struct sockaddr_in _addr{};
        std::function<void(const char *ip, const Message *msg)> _receiveCallback;
    };
}
#endif //YUXIANTECH_PIANOLESSON_BROADCAST_H
