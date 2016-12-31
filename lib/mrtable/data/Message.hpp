#ifndef __MESSAGE_HPP__
#define __MESSAGE_HPP__

#include <string>
#include <tuio/TuioTime.h>

namespace mrtable {
    namespace data {
        class Message {
            public:
                int cmdCode;
                string params;
                TUIO::TuioTime time;
                Message(int code, string paramString, TUIO::TuioTime msgTime) : cmdCode(code), params(paramString), time(msgTime) {
                }
        };
    }
}

#endif
