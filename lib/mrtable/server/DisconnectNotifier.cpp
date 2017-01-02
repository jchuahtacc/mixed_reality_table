#ifndef __DISCONNECTNOTIFIER_CPP__
#define __DISCONNECTNOTIFIER_CPP__

#include <boost/asio.hpp>
#include "DisconnectNotifier.h"
#include "CommandServer.h"

using boost::asio::ip::tcp;

namespace mrtable {
    namespace server {
        CommandServer* DisconnectNotifier::server_ = NULL;
        void DisconnectNotifier::disconnect(tcp::socket* socket) {
            server_->disconnect(socket);
        }

        void DisconnectNotifier::put(CommandServer* server) {
            server_ = server;
        }
    }
}


#endif
