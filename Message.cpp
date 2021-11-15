//
// Created by Herbert on 2021/11/14.
//

#include <cstdio>
#include <cstring>
#include "../libs/cjson/cJSON.h"
#include "../libs/minilzo/minilzo.h"

#include "Message.h"

namespace PianoLesson {

#define HEAP_ALLOC(var, size) \
    lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]

    static HEAP_ALLOC(wrkmem, LZO1X_1_MEM_COMPRESS);

    Message::Message() {
        lzo_init();

        _json = cJSON_CreateObject();
    }

    Message::Message(const char *msg) {
        lzo_init();

        char out[1024] = {};
        auto len = (lzo_uint) sizeof(out);
        auto r = lzo1x_decompress(reinterpret_cast<const unsigned char *>(msg), len,
                                  reinterpret_cast<unsigned char *>(out), &len, nullptr);
        if (r != LZO_E_OK) {
            /* this should NEVER happen */
            fprintf(stderr, "[Message][Message]internal error - decompression failed: %d\n", r);
            _valid = false;
            return;
        }

        _json = cJSON_Parse(out);

        if (!_json) {
            _valid = false;
            fprintf(stderr, "[Message][Message]parse message failed: %s\n", out);
        }
    }

    Message::~Message() {
        cJSON_Delete(_json);
    }

    bool Message::valid() const {
        return _valid;
    }

    void Message::setType(MessageType type) const {
        setInteger("messageType", type);
    }

    MessageType Message::getType() const {
        return static_cast<MessageType>(getInteger("messageType", MessageType::Unknown));
    }

    const char *Message::getString(const char *name) const {
        if (!_json) return nullptr;
        auto object = cJSON_GetObjectItem(_json, name);
        return cJSON_GetStringValue(object);
    }

    int Message::getInteger(const char *name, int defaultValue) const {
        if (!_json) return defaultValue;
        auto object = cJSON_GetObjectItem(_json, name);
        return object == nullptr ? defaultValue : (int) cJSON_GetNumberValue(object);
    }

    void Message::setString(const char *name, const char *value) const {
        cJSON_AddStringToObject(_json, name, value);
    }

    void Message::setInteger(const char *name, int value) const {
        char str[32] = {};
        sprintf(str, "%d", value);
        cJSON_AddStringToObject(_json, name, str);
    }

    const char *Message::stringify() const {

        if (strlen(_buffer) > 0) return _buffer;

        auto str = cJSON_PrintUnformatted(_json);
        auto len = (lzo_uint) sizeof(_buffer);
        auto r = lzo1x_1_compress(reinterpret_cast<const unsigned char *>(str), strlen(str),
                                  (unsigned char *) (_buffer), &len, wrkmem);
        if (r != LZO_E_OK) {
            /* this should NEVER happen */
            fprintf(stderr, "[Message][Message]internal error - compression failed: %d\n", r);
            return nullptr;
        }

        return _buffer;
    }
}
