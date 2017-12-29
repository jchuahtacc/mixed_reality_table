
#include "TuioWrapper.h"
#include "StdErrSender.h"
#include "../config/config.h"

using namespace mrtable::config;

namespace mrtable_tuio {

    bool TuioWrapper::running = false;
    shared_ptr< TUIO::TuioServer > TuioWrapper::tuio_server = nullptr;
    shared_ptr< ContainerPtrQueue > TuioWrapper::queue = nullptr;

    bool TuioWrapper::init(shared_ptr< ContainerPtrQueue > container_queue) {
        TuioWrapper::queue = container_queue;
        TUIO::OscSender* sender;
        sender = new mrtable_tuio::StdErrSender(mrtable_tuio::STDERRSENDER_SUMMARY);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        TuioWrapper::tuio_server = shared_ptr< TUIO::TuioServer >( new TUIO::TuioServer(sender) );
        if (ServerConfig::enable_udp) {
            sender = new TUIO::UdpSender(ServerConfig::host.c_str(), ServerConfig::udp_port);
            tuio_server->addOscSender(sender);
        }
        if (ServerConfig::enable_tcp) {
            sender = new TUIO::TcpSender(ServerConfig::tcp_port);
            tuio_server->addOscSender(sender);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (ServerConfig::enable_web) {
            sender = new TUIO::WebSockSender(ServerConfig::web_port);
            tuio_server->addOscSender(sender);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        TuioWrapper::running = true;

        return true;
    }

    void TuioWrapper::put(shared_ptr< TUIO::TuioContainer > containerPtr) {
        queue->push(containerPtr);
    }

    void TuioWrapper::stop() {
        TuioWrapper::running = false;
    }

    void TuioWrapper::worker() {
        while (TuioWrapper::running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}
