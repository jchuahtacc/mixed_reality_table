#ifndef __COMMANDSESSION_HPP__
#define __COMMANDSESSION_HPP__


#include <cstdlib>
#include <iostream>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/lockfree/queue.hpp>
#include <string>

using boost::asio::ip::tcp;

namespace mrtable {
    namespace server {
        class CommandSession {
            public: 
                CommandSession(boost::asio::io_service& io_service, MutexQueue<string>* msgQueue) : socket_(io_service), messages(msgQueue) {
                }

                tcp::socket& socket() {
                    return socket_;
                }

                void start() {
                    std::cerr << "Command Session started" << std::endl;
                    socket_.async_read_some(boost::asio::buffer(data_, max_length),
                        boost::bind(&CommandSession::handle_read, this,
                          boost::asio::placeholders::error,
                          boost::asio::placeholders::bytes_transferred));
                }

                void handle_read(const boost::system::error_code& error, size_t bytes_transferred) {
                    if (!error) {
                        std::string str(data_);
                        messages->push(str);
                        // do stuff with good command
                    } else {
                        std::cerr << "Command Session terminated: " << error.message() << std::endl;
                        delete this;
                    }
                }

            private:
                tcp::socket socket_;
                MutexQueue<string>* messages; 
                enum { max_length = 1024 };
                char data_[max_length];
        };
    }
}

#endif
