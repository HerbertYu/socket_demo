//
// Created by Herbert on 2021/11/14.
//

#ifndef YUXIANTECH_PIANOLESSON_MESSAGETYPE_H
#define YUXIANTECH_PIANOLESSON_MESSAGETYPE_H

namespace PianoLesson {

    enum MessageType {
        Unknown,

        BroadcastBegin = 0x10000,
        BroadcastServerOnline,
        BroadcastClientOnline,

        P2PBegin = 0x20000,
        P2PHeartbeat,
    };
}

#endif //YUXIANTECH_PIANOLESSON_MESSAGETYPE_H
