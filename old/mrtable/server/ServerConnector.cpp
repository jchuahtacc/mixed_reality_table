#ifndef __SERVERCONNECTOR_CPP__
#define __SERVERCONNECTOR_CPP__

#include <boost/asio.hpp>
#include "ServerConnector.h"
#include "CommandServer.h"

using boost::asio::ip::tcp;

namespace mrtable {
    namespace server {
        CommandServer* ServerConnector::server_ = NULL;
        void ServerConnector::disconnect(tcp::socket* socket) {
            server_->disconnect(socket);
        }

        void ServerConnector::put(CommandServer* server) {
            server_ = server;
        }
    }
}


#endif
