//
// Created by Herbert on 2021/11/13.
//
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>

#include <utility>

#include "Message.h"
#include "Broadcast.h"

namespace PianoLesson {

    Broadcast::Broadcast(const char *ip, int port) {
        fprintf(stdout, "[Message][Broadcast]broadcast created\n");

        init(ip, port);
    }

    Broadcast::~Broadcast() {
        _started = false;
        close(_recvSockFd);
        close(_sendSockFd);

        _listen.join();
        fprintf(stdout, "[Message][Broadcast]broadcast destroyed\n");
    }

    void Broadcast::setCallback(std::function<void(const char *, const Message *)> callback) {
        _receiveCallback = std::move(callback);
    }

    bool Broadcast::send(const char *msg) const {
        fprintf(stdout, "[Message][Broadcast]send message:%s\n", msg);
        int ret = sendto(_sendSockFd, msg, strlen(msg) + 1, 0, (struct sockaddr *) &_addr, sizeof(struct sockaddr_in));

        if (ret <= 0) {
            fprintf(stderr, "[Message][Broadcast]send message error:%s\n", strerror(errno));
        }
        return ret > 0;
    }

    bool Broadcast::send(const Message *message) const {
        return send(message->stringify());
    }

    bool Broadcast::init(const char *ip, int port) {
        {
            fprintf(stdout, "[Message][Broadcast]init send socket\n");
            _sendSockFd = socket(AF_INET, SOCK_DGRAM, 0);
            if (_sendSockFd < 0) {
                fprintf(stderr, "[Message][Broadcast]init socket error:%s\n", strerror(errno));
                return false;
            }

            _addr.sin_family = AF_INET;
            _addr.sin_port = htons(port);
            _addr.sin_addr.s_addr = inet_addr(ip);

            int broad_flag = 1;
            setsockopt(_sendSockFd, SOL_SOCKET, SO_BROADCAST, &broad_flag, sizeof(int));
        }
        {
            fprintf(stdout, "[Message][Broadcast]init receive socket\n");
            struct sockaddr_in addr{};
            _recvSockFd = socket(AF_INET, SOCK_DGRAM, 0);
            if (_recvSockFd < 0) {
                fprintf(stderr, "[Message][Broadcast]init socket error:%s\n", strerror(errno));
                return false;
            }

            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            addr.sin_addr.s_addr = htonl(INADDR_ANY);
            fprintf(stdout, "[Message][Broadcast]bind receive socket\n");
            int ret = bind(_recvSockFd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));
            if (ret < 0) {
                fprintf(stderr, "[Message][Broadcast]bind error:%s\n", strerror(errno));
                return false;
            }
        }
        _started = true;

        _listen = std::thread(&Broadcast::listen, this);
        return true;
    }

    void Broadcast::listen() const {
        struct sockaddr_in sendaddr{};
        socklen_t len;
        while (_started) {
            char buf[1024] = {0};
            fprintf(stdout, "[Message][Broadcast]listen for any broadcast\n");
            int ret = recvfrom(_recvSockFd, buf, sizeof(buf), 0, (struct sockaddr *) &sendaddr, &len);
            if (ret < 0) {
                fprintf(stderr, "[Message][Broadcast]recvfrom error:%s\n", strerror(errno));
                sleep(10);
                continue;
            }
            auto ip = inet_ntoa(sendaddr.sin_addr);
            fprintf(stdout, "[Message][Broadcast]receive from %s : %s\n", ip, buf);
            Message message(buf);
            if (message.valid() && _receiveCallback) {
                _receiveCallback(ip, &message);
            }
        }
    }
}
