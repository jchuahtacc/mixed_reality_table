#ifndef __COMMANDSERVER_HPP__
#define __COMMANDSERVER_HPP__

#include <cstdlib>
#include <iostream>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/lockfree/queue.hpp>
#include "CommandSession.hpp"

using boost::asio::ip::tcp;

namespace mrtable {
    namespace server {
        class CommandServer {
            public:
                CommandServer(boost::asio::io_service& io_service, short port) : io_service_(io_service), acceptor_(io_service, tcp::endpoint(tcp::v4(), port)) {
                    CommandSession* new_session = new CommandSession(io_service_);
                    acceptor_.async_accept(new_session->socket(), boost::bind(&CommandServer::handle_accept, this, new_session, boost::asio::placeholders::error));
                }

                void handle_accept(CommandSession* new_session, const boost::system::error_code& error) {
                    if (!error) {
                        std::cerr << "Command Server accepted connection" << std::endl;
                        new_session->start();
                        new_session = new CommandSession(io_service_);
                        // Handle another connection request after starting this session
                        acceptor_.async_accept(new_session->socket(), boost::bind(&CommandServer::handle_accept, this, new_session, boost::asio::placeholders::error));
                    } else {
                        std::cerr << "Error while accepting connection: " << error.message() << std::endl;
                        delete new_session;
                    }
                }

            private:
                boost::asio::io_service& io_service_;
                tcp::acceptor acceptor_;
        };
    }
}

#endif
