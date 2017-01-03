#ifndef __SERVERCONNECTOR_H__
#define __SERVERCONNECTOR_H__

#include "CommandServer.h"
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

namespace mrtable {
    namespace server {
        class ServerConnector {
            public:
                static void disconnect(tcp::socket*);
                static void put(CommandServer* server);
                static CommandServer* server_;
        };
    }
}

#endif
