#ifndef __DISCONNECTNOTIFIER_H__
#define __DISCONNECTNOTIFIER_H__

#include "CommandServer.h"
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

namespace mrtable {
    namespace server {
        class DisconnectNotifier {
            public:
                static void disconnect(tcp::socket*);
                static void put(CommandServer* server);
                static CommandServer* server_;
        };
    }
}

#endif
