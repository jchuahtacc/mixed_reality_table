#include <iostream>
#include <rpi_motioncam/RPiMotionCam.h>
#include "tbb/concurrent_queue.h"
#include "mrtable_tuio/StdErrSender.h"
#include "config/config.h"
#include <tuio/TuioServer.h>
#include <memory>
#include <thread>
#include <chrono>

using namespace std;
using namespace tbb;
using namespace mrtable::config;

shared_ptr< TUIO::TuioServer > tuio_server = nullptr;

void init_tuio() {
    TUIO::OscSender* sender;
    sender = new mrtable_tuio::StdErrSender(mrtable_tuio::STDERRSENDER_SUMMARY);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    tuio_server = shared_ptr< TUIO::TuioServer >( new TUIO::TuioServer(sender) );
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
}


int main(int argc, const char* argv[]) {
    cout << "Initializing TUIO Server" << endl;
    init_tuio();
    cout << "Initialized TUIO Server" << endl;
    cout << "Shutting down" << endl;

}
