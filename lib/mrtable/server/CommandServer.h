#ifndef __COMMANDSERVER_HPP__
#define __COMMANDSERVER_HPP__

#include "../data/data.hpp"
#include <cstdlib>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include "CommandSession.h"

using boost::asio::ip::tcp;
using namespace mrtable::data;

namespace mrtable {
    namespace server {
        class CommandServer {
            public:
                CommandServer(boost::asio::io_service& io_service, cv::Ptr< mrtable::data::MutexQueue<string> > msgQueue, cv::Ptr< mrtable::data::MutexQueue<string> > sendQueue, short port);
                ~CommandServer();

                void handle_accept(CommandSession* new_session, const boost::system::error_code& error);
                void disconnect(tcp::socket*);
                void handle_write(const boost::system::error_code& error, size_t bytes_transferred);
                void broadcast(string);

            private:
                cv::Ptr< MutexQueue<string> > msgQueue_, sendQueue_;
                boost::asio::io_service& io_service_;
                tcp::acceptor acceptor_;
                vector< tcp::socket* > sockets;
        };
    }
}

#endif
