#ifndef __COMMANDSESSION_HPP__
#define __COMMANDSESSION_HPP__

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
        class CommandSession {
            public: 
                CommandSession(boost::asio::io_service& io_service, Ptr< mrtable::data::MutexQueue<string> > msgQueue, Ptr< mrtable::data::MutexQueue<string> > sendQueue);

                ~CommandSession();

                tcp::socket& socket();

                void bindSocket();

                void start();

                void do_nothing(const boost::system::error_code& error, size_t bytes_transferred);

                void handle_read(const boost::system::error_code& error, size_t bytes_transferred);

            private:
                tcp::socket socket_;
                Ptr< MutexQueue<string> > msgQueue_; 
                Ptr< MutexQueue<string> > sendQueue_;
                vector< string >* sends = NULL;
                enum { max_length = 1024 };
                char data_[max_length];
                char send_[max_length];
        };
    }
}

#endif
