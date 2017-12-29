#include "StdErrSender.h"
#include <iostream>

using namespace TUIO;
using namespace std;

namespace mrtable_tuio {
    StdErrSender::StdErrSender(STDERRSENDER_LEVEL_T level) : level_(level) {
        buffer_size = 1024;
        local = true;
    }

    StdErrSender::~StdErrSender() {
    }

    bool StdErrSender::sendOscPacket(osc::OutboundPacketStream *bundle) {
        if (level_ > STDERRSENDER_SILENT) {
            cerr << "TUIO Osc Packet Sent (" << bundle->Size() << " bytes)" << endl;
        }
        if (level_ > STDERRSENDER_SUMMARY) {
            cerr << "Osc Packet Data: " << endl;
            cerr << bundle->Data() << endl;
        }
        return true;
    }
}
