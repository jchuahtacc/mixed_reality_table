#ifndef __COMMANDSERVER_CPP__
#define __COMMANDSERVER_CPP__

#include <cstdlib>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include "CommandServer.h"
#include "CommandSession.h"
#include "DisconnectNotifier.h"

using boost::asio::ip::tcp;
using namespace std;
using namespace mrtable::data;

namespace mrtable {
    namespace server {
        CommandServer::CommandServer(boost::asio::io_service& io_service, cv::Ptr< mrtable::data::MutexQueue<string> > msgQueue, cv::Ptr< mrtable::data::MutexQueue<string> > sendQueue, short port) : io_service_(io_service), acceptor_(io_service, tcp::endpoint(tcp::v4(), port)), msgQueue_(msgQueue), sendQueue_(sendQueue) {

            DisconnectNotifier::put(this);
            CommandSession* new_session = new CommandSession(io_service_, msgQueue, sendQueue);
            acceptor_.async_accept(new_session->socket(), boost::bind(&CommandServer::handle_accept, this, new_session, boost::asio::placeholders::error));
        }

        CommandServer::~CommandServer() {
            msgQueue_.release();
        }

        void CommandServer::handle_accept(CommandSession* new_session, const boost::system::error_code& error) {
            if (!error) {
                sockets.push_back(&(new_session->socket()));
                std::cerr << "Command Server accepted connection" << std::endl;
                new_session->start();
                new_session = new CommandSession(io_service_, msgQueue_, sendQueue_);
                // Handle another connection request after starting this session
                acceptor_.async_accept(new_session->socket(), boost::bind(&CommandServer::handle_accept, this, new_session, boost::asio::placeholders::error));
            } else {
                std::cerr << "Error while accepting connection: " << error.message() << std::endl;
                delete new_session;
            }
        }

        void CommandServer::disconnect(tcp::socket* socket) {
            for (vector<tcp::socket*>::iterator it = sockets.begin(); it < sockets.end(); it++) {
                if (*it == socket) {
                    sockets.erase(it);
                }
            }
            std::cout << "Socket disconnected. " << sockets.size() << " currently connected." << std::endl;
        }

        void CommandServer::handle_write(const boost::system::error_code& error, size_t bytes_transferred) {
            std::cout << "Message sent " << std::endl;
        }

        void CommandServer::broadcast(string message) {
            for (vector<tcp::socket*>::iterator socket = sockets.begin(); socket < sockets.end(); socket++) {
                int length = strlen(message.c_str());
                boost::asio::async_write(**socket, boost::asio::buffer(message.c_str(), length),
                    boost::bind(&CommandServer::handle_write, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
            }
        }
    }
}

#endif
