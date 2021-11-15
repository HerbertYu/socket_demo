//
//  Client.cpp
//  PianoLesson
//
//  Created by Herbert on 2021/11/13.
//

#include <arpa/inet.h>//包含socket函数使用的各种协议族，send(),recv()
#include <unistd.h>//调用linux系统函数的头文件(read(),write(),send()，select())
#include <thread>

#include "LocalNet.h"
#include "Broadcast.h"
#include "Message.h"
#include "MessageType.h"
#include "Client.h"

#define BUFFER_SIZE 1024

namespace PianoLesson {

    Client::Client() {
        fprintf(stdout, "[Message][Client]client created\n");
        _localNet = new LocalNet();
        _broadcast = new Broadcast(_localNet->broadcastAddr(), _localNet->broadcastPort());
        _broadcast->setCallback(std::bind(
                &Client::onReceiveBroadcast,
                this,
                std::placeholders::_1,
                std::placeholders::_2
        ));
        broadcastOnline();
    }

    Client::~Client() {
        _started = false;
        close(_sockfd);

        _receive.join();

        delete _broadcast;
        delete _localNet;
        fprintf(stdout, "[Message][Client]client destroyed\n");
    }

    bool Client::connect(const char *ip, int port) {
        fprintf(stdout, "[Message][Client]init socket\n");

        ///定义sockfd
        _sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (_sockfd < 0) {
            fprintf(stderr, "[Message][Client]init socket error:%s\n", strerror(errno));
            return false;
        }

        ///定义sockaddr_in
        struct sockaddr_in servaddr{};
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port);  ///服务器端口
        servaddr.sin_addr.s_addr = inet_addr(ip);  ///服务器ip

        fprintf(stdout, "[Message][Client]connect %s:%d\n", ip, port);
        //连接服务器，成功返回0，错误返回-1
        if (::connect(_sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
            fprintf(stdout, "[Message][Client]connect error\n");
            return false;
        }

        _started = true;
        fprintf(stdout, "[Message][Client]start receive thread\n");
        _receive = std::thread(&Client::receive, this);

        return true;
    }

    void Client::receive() {
        struct timeval tv{};
        /*设置超时时间*/
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        fd_set rfds;
        int retval, maxfd;

        while (_started) {
            /*把可读文件描述符的集合清空*/
            FD_ZERO(&rfds);
            maxfd = 0;
            /*把当前连接的文件描述符加入到集合中*/
            FD_SET(_sockfd, &rfds);
            /*找出文件描述符集合中最大的文件描述符*/
            if (maxfd < _sockfd)
                maxfd = _sockfd;
            /*等待聊天*/
            retval = select(maxfd + 1, &rfds, nullptr, nullptr, &tv);
            if (retval > 0) {

                /*服务器发来了消息*/
                if (FD_ISSET(_sockfd, &rfds)) {
                    char recvbuf[BUFFER_SIZE];
                    recv(_sockfd, recvbuf, sizeof(recvbuf), 0);
                    fprintf(stdout, "[Message][Client]receive:%s\n", recvbuf);

                    Message message(recvbuf);
                    if (message.valid())
                        handleMessage(&message);
                }
            }
        }
    }

    void Client::send(const char *msg) const {
        fprintf(stdout, "[Message][Client]send message:%s\n", msg);
        auto len = ::send(_sockfd, msg, strlen(msg) + 1, 0);
        if (len <= 0) {
            fprintf(stderr, "[Message][Client]send message error:%s\n", strerror(errno));
        }
    }

    void Client::broadcast(const char *msg) const {
        fprintf(stdout, "[Message][Client]broadcast message:%s\n", msg);
        _broadcast->send(msg);
    }

    void Client::broadcastOnline() const {
        Message message;
        message.setType(MessageType::BroadcastClientOnline);
        message.setString("ip", _localNet->ipAddr());
        message.setString("mac", _localNet->macAddr());

        fprintf(stdout, "[Message][Client]broadcastOnline ip:%s, mac:%s\n", _localNet->ipAddr(), _localNet->macAddr());
        broadcast(message.stringify());
    }

    void Client::handleMessage(const Message *message) {
        auto messageType = message->getType();
        switch (messageType) {

            default:
                break;
        }
    }

    void Client::onReceiveBroadcast(const char *ip, const Message *message) {
        auto messageType = message->getType();
        switch (messageType) {
            case BroadcastServerOnline: {
                auto mac = message->getString("mac");
                auto port = message->getInteger("port");
                fprintf(stdout, "[Message][Client]onReceiveBroadcast ip:%s, port:%d, mac:%s\n", ip, port, mac);
                connect(ip, port);
                break;
            }
            default:
                break;
        }
    }

}
