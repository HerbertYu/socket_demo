//
//  Server.hpp
//  PianoLesson
//
//  Created by Herbert on 2021/11/13.
//

#ifndef YUXIANTECH_PIANOLESSON_SERVER_H
#define YUXIANTECH_PIANOLESSON_SERVER_H

#include <map>
#include <functional>
#include <thread>

namespace PianoLesson {
    class LocalNet;

    class Broadcast;

    class Message;

    class Terminal;

    class Server {
    public:
        Server();

        virtual ~Server();

        void send(int sockfd, const char *msg) const;

        void sendAll(const char *msg) const;

        void broadcast(const char *msg) const;

    private:
        bool serve();

        void connect();

        void receive();

        void broadcastOnline() const;

        void handleMessage(int sockfd, const Message *message);

        void onReceiveBroadcast(const char* ip, const Message* message);

        void checkHeartbeat();

    private:
        bool _started{false};
        int _sockfd{-1};
        Broadcast *_broadcast;
        LocalNet *_localNet;
        std::thread _connect;
        std::thread _receive;
        std::map<std::string, std::string> _onlineTerminals;
        std::map<int, Terminal*> _clients;
        std::function<void(int, const Message *)> _receiveCallback;
    };
}

#endif /* YUXIANTECH_PIANOLESSON_SERVER_H */
