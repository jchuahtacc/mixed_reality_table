#ifndef __COMMANDSESSION_CPP__
#define __COMMANDSESSION_CPP__

#include "ServerConnector.h"
#include "CommandSession.h"
#include <cstdlib>
#include <iostream>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/lockfree/queue.hpp>
#include <string>
#include <opencv2/core/core.hpp>

using boost::asio::ip::tcp;
using namespace mrtable::data;

namespace mrtable {
    namespace server {
        CommandSession::CommandSession(boost::asio::io_service& io_service, Ptr< mrtable::data::MutexQueue<string> > msgQueue, Ptr< mrtable::data::MutexQueue<string> > sendQueue) : socket_(io_service), msgQueue_(msgQueue), sendQueue_(sendQueue) {
            sends = new vector< string >();
        }

        CommandSession::~CommandSession() {
            msgQueue_.release();
            sendQueue_.release();
        }

        tcp::socket& CommandSession::socket() {
            return socket_;
        }

        void CommandSession::bindSocket() {
             socket_.async_read_some(boost::asio::buffer(data_, max_length),
                boost::bind(&CommandSession::handle_read, this,
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
       }

        void CommandSession::start() {
            std::cerr << "Command Session started" << std::endl;
            bindSocket();
        }

        void CommandSession::do_nothing(const boost::system::error_code& error, size_t bytes_transferred) {
            if (!error) {
                std::cout << "Sent response" << std::endl;
            } else {
                std::cout << "Error sending response: " << error.message() << std::endl;
            }
        }

        void CommandSession::handle_read(const boost::system::error_code& error, size_t bytes_transferred) {
            if (!error) {
                if (sendQueue_->popAll(sends)) {
                    std::cout << "Popped messages to send " << std::endl;
                    for (vector< string >::iterator send = sends->begin(); send < sends->end(); send++) {
                        std::cout << "Sending " << *send << std::endl;
                        boost::asio::async_write(socket_, boost::asio::buffer(send->c_str(), max_length), 
                          boost::bind(&CommandSession::do_nothing, this,
                          boost::asio::placeholders::error,
                          boost::asio::placeholders::bytes_transferred));
                    }
                };
                std::string str(data_);
                memset(data_, 0, 1024);
                msgQueue_->push(str);
                bindSocket();
            } else {
                if (error == boost::asio::error::eof || error == boost::asio::error::connection_reset) {
                    ServerConnector::disconnect(&socket_);
                }
                delete this;
            }
        }
    }
}

#endif
