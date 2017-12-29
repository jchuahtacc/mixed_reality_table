#ifndef __STDERRSENDER_H__
#define __STDERRSENDER_H__

#include <tuio/OscSender.h>
#include <tuio/osc/OscOutboundPacketStream.h>

/**
 An OSC Sender that dumps data to stderr
 */

using namespace TUIO;

namespace mrtable_tuio {
    typedef enum STDERRSENDER_LEVEL_T {
        STDERRSENDER_SILENT,
        STDERRSENDER_SUMMARY,
        STDERRSENDER_VERBOSE
    } STDERRSENDER_LEVEL_T;
    class StdErrSender : public OscSender {
        public:
            StdErrSender(STDERRSENDER_LEVEL_T level = STDERRSENDER_SILENT);
            ~StdErrSender();
            bool sendOscPacket(osc::OutboundPacketStream *bundle);
            const char* tuio_type() { return "TUIO/STDERR"; }
            bool isConnected() { return true; }
        private:
            STDERRSENDER_LEVEL_T level_;
    };
}

#endif
