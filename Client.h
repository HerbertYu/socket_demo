//
//  Client.hpp
//  PianoLesson
//
//  Created by Herbert on 2021/11/13.
//

#ifndef YUXIANTECH_PIANOLESSON_CLIENT_H
#define YUXIANTECH_PIANOLESSON_CLIENT_H

#include <thread>
#include <functional>
#include "Terminal.h"

namespace PianoLesson {

    class LocalNet;

    class Broadcast;

    class Message;

    class Client {
    public:
        Client();

        virtual ~Client();

        bool connect(const char *ip, int port);

        void send(const char *msg) const;

        void broadcast(const char *msg) const;

    private:
        void receive();

        void broadcastOnline() const;

        void handleMessage(const Message *message);

        void onReceiveBroadcast(const char* ip, const Message* message);

        void sendHeartbeat();

    private:
        int _sockfd{-1};
        bool _started{false};
        LocalNet *_localNet;
        Broadcast *_broadcast;
        std::thread _receive;
        std::function<void(const Message *)> _receiveCallback;
    };
}
#endif /* YUXIANTECH_PIANOLESSON_CLIENT_H */
