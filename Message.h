//
// Created by Herbert on 2021/11/14.
//

#ifndef YUXIANTECH_PIANOLESSON_MESSAGE_H
#define YUXIANTECH_PIANOLESSON_MESSAGE_H

#include "MessageType.h"

class cJSON;

namespace PianoLesson {

    class Message {
    public:
        Message();

        explicit Message(const char *msg);

        virtual ~Message();

        bool valid() const;

        const char *stringify() const;

        const char *getString(const char *name) const;

        int getInteger(const char *name, int defaultValue = 0) const;

        void setString(const char *name, const char *value) const;

        void setInteger(const char *name, int value) const;

        MessageType getType() const;

        void setType(MessageType type) const;

    private:
        cJSON *_json{nullptr};
        bool _valid{true};
        char _buffer[1024] = {};
    };
}

#endif //YUXIANTECH_PIANOLESSON_MESSAGE_H
