//
//  Terminal.cpp
//  PianoLesson
//
//  Created by Herbert on 2021/11/13.
//


#include "Terminal.h"

namespace PianoLesson {
    Terminal::Terminal(int sockfd, const char *ip, const char *mac)
            : _sockfd(sockfd), _ip(ip), _mac(mac) {

    }

    Terminal::~Terminal() {

    }

    const char *Terminal::ip() const {
        return _ip.c_str();
    }

    const char *Terminal::mac() const {
        return _mac.c_str();
    }
}
