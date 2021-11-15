//
//  Server.cpp
//  PianoLesson
//
//  Created by Herbert on 2021/11/13.
//
#include <arpa/inet.h>
#include <unistd.h>

#include "LocalNet.h"
#include "Broadcast.h"
#include "Message.h"
#include "Terminal.h"
#include "MessageType.h"
#include "Server.h"

#define PORT 7259
#define IP "127.0.0.1"
#define BUFFER_SIZE 1024

namespace PianoLesson {
    Server::Server() {
        fprintf(stdout, "[Message][Server]server created\n");
        _localNet = new LocalNet();
        _broadcast = new Broadcast(_localNet->broadcastAddr(), _localNet->broadcastPort());
        _broadcast->setCallback(std::bind(
                &Server::onReceiveBroadcast,
                this,
                std::placeholders::_1,
                std::placeholders::_2
        ));
        serve();
    }

    Server::~Server() {
        _started = false;
        close(_sockfd);

        for (auto it : _clients) {
            close(it.first);
            delete it.second;
        }
        _connect.join();
        _receive.join();
        delete _broadcast;
        delete _localNet;
        fprintf(stdout, "[Message][Server]server destroyed\n");
    }

    bool Server::serve() {
        fprintf(stdout, "[Message][Server]init socket\n");
        _sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (_sockfd < 0) {
            fprintf(stderr, "[Message][Client]init socket error:%s\n", strerror(errno));
            return false;
        }

        struct sockaddr_in sockaddr{};
        sockaddr.sin_family = AF_INET;
        sockaddr.sin_port = htons(PORT);
        sockaddr.sin_addr.s_addr = inet_addr(IP);

        fprintf(stdout, "[Message][Server]bind socket\n");
        if (bind(_sockfd, (struct sockaddr *) &sockaddr, sizeof(sockaddr)) == -1) {
            fprintf(stderr, "[Message][Server]bind error:%s\n", strerror(errno));
            return false;
        }

        fprintf(stdout, "[Message][Server]listen socket\n");
        if (listen(_sockfd, 20) == -1) {
            fprintf(stderr, "[Message][Server]listen error:%s\n", strerror(errno));
            return false;
        }

        _started = true;
        fprintf(stdout, "[Message][Server]start connect thread\n");
        _connect = std::thread(&Server::connect, this);
        fprintf(stdout, "[Message][Server]start receive thread\n");
        _receive = std::thread(&Server::receive, this);

        broadcastOnline();
        return true;
    }

    void Server::connect() {
        while (_started) {
            struct sockaddr_in sockaddr{};
            socklen_t len;

            fprintf(stdout, "[Message][Server]wait for connections\n");
            auto sockfd = accept(_sockfd, (struct sockaddr *) &sockaddr, &len);
            if (sockfd < 0) {
                fprintf(stderr, "[Message][Server]connect error:%s\n", strerror(errno));
                continue;
            }

            auto ip = inet_ntoa(sockaddr.sin_addr);
            auto found = std::find_if(
                    _onlineTerminals.begin(),
                    _onlineTerminals.end(),
                    [ip](const std::pair<std::string, std::string> &it) {
                        return it.second == ip;
                    }
            );
            if (found != _onlineTerminals.end()) {
                auto terminal = new Terminal(sockfd, found->second.c_str(), found->first.c_str());
                fprintf(stdout, "[Message][Server]record connected terminal:ip:%s, mac:%s\n", found->second.c_str(),
                        found->first.c_str());
                _clients[sockfd] = terminal;
            } else {
                fprintf(stderr, "[Message][Server]unknown terminal:%s\n", ip);
                // TODO: close socket and next?
            }
        }
    }

    void Server::receive() {
        struct timeval tv{};
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        while (_started) {
            for (auto &_client : _clients) {
                auto sockfd = _client.first;
                fd_set rfds;
                FD_ZERO(&rfds);
                int maxfd = 0;
                int retval = 0;
                FD_SET(sockfd, &rfds);
                if (maxfd < sockfd) {
                    maxfd = sockfd;
                }
                retval = select(maxfd + 1, &rfds, nullptr, nullptr, &tv);
                if (retval > 0) {
                    char buf[BUFFER_SIZE];
                    memset(buf, 0, sizeof(buf));
                    recv(sockfd, buf, sizeof(buf), 0);

                    fprintf(stdout, "[Message][Server]receive from %d : %s\n", sockfd, buf);

                    Message message(buf);
                    if (message.valid())
                        handleMessage(sockfd, &message);
                }
            }
            sleep(10);
        }
    }

    void Server::send(int sockfd, const char *msg) const {
        fprintf(stdout, "[Message][Server]send message to %s:%s\n", _clients.at(sockfd)->ip(), msg);
        ::send(sockfd, msg, strlen(msg) + 1, 0);
    }

    void Server::sendAll(const char *msg) const {
        fprintf(stdout, "[Message][Server]send message to all:%s\n", msg);
        for (auto &_client : _clients) {
            auto sockfd = _client.first;
            ::send(sockfd, msg, strlen(msg) + 1, 0);
        }
    }

    void Server::broadcast(const char *msg) const {
        fprintf(stdout, "[Message][Server]broadcast message:%s\n", msg);
        _broadcast->send(msg);
    }

    void Server::broadcastOnline() const {
        Message message;
        message.setType(MessageType::BroadcastServerOnline);
        message.setString("ip", _localNet->ipAddr());
        message.setString("mac", _localNet->macAddr());
        message.setInteger("port", PORT);
        fprintf(stdout, "[Message][Server]broadcastOnline ip:%s, port:%d, mac:%s\n", _localNet->ipAddr(), PORT,
                _localNet->macAddr());
        broadcast(message.stringify());
    }

    void Server::handleMessage(int sockfd, const Message *message) {
        auto messageType = message->getType();
        switch (messageType) {

            default:
                break;
        }
    }

    void Server::onReceiveBroadcast(const char *ip, const Message *message) {
        auto messageType = message->getType();
        switch (messageType) {
            case BroadcastClientOnline: {
                auto mac = message->getString("mac");
                auto ip = message->getString("ip");
                fprintf(stdout, "[Message][Server]onReceiveBroadcast ip:%s, mac:%s\n", ip, mac);
                _onlineTerminals[mac] = ip;
                broadcastOnline();
                break;
            }

            default:
                break;
        }
    }
}
