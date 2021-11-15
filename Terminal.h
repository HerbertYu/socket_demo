//
//  Terminal.hpp
//  PianoLesson
//
//  Created by Herbert on 2021/11/13.
//

#ifndef YUXIANTECH_PIANOLESSON_TERMINAL_H
#define YUXIANTECH_PIANOLESSON_TERMINAL_H

#include <string>

namespace PianoLesson {
    class Terminal {
    public:
        explicit Terminal(int sockfd, const char *ip, const char *mac);

        virtual ~Terminal();

        const char* ip() const;
        const char* mac() const;

        void updateTouchTime();

    private:
        int _sockfd{-1};
        std::string _ip;
        std::string _mac;
        int _lastTouchTime{0};
    };
}
#endif /* YUXIANTECH_PIANOLESSON_TERMINAL_H */
