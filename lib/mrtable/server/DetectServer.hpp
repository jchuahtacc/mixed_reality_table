#ifndef __DETECTSERVER_HPP__
#define __DETECTSERVER_HPP__

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <tuio/TuioServer.h>
#include <chrono>
#include <iostream>
#include <thread>

using namespace cv;
using namespace mrtable::data;
using namespace mrtable::process;
using namespace mrtable::server;
using namespace mrtable::sources;

namespace mrtable {
    namespace server {
        class DetectServer {
            public:
                DetectServer(cv::Ptr< mrtable::config::ServerConfig > config);
                ~DetectServer();
                int start();
                void setVideoSource(cv::Ptr< mrtable::sources::VideoSource > videoSource);
                void setPreview(bool preview);

            private:
                void initServer();
                void initProcessQueue();
                Ptr< mrtable::config::ServerConfig > config_;
                Ptr< mrtable::sources::VideoSource > vidSource;
                ProcessQueue* proc = NULL;
                TUIO::TuioServer* server = NULL;
                Ptr< MutexQueue<string> > msgQueue;
                bool preview_ = false;
                bool preview_added = false;

        };

        DetectServer::DetectServer(Ptr< mrtable::config::ServerConfig > config) : config_(config) {
            SharedData::put(KEY_CONFIG, config_);
            msgQueue = new MutexQueue<string>();
            SharedData::put(KEY_MSG_QUEUE, msgQueue);
            initServer();
            initProcessQueue();
        }

        DetectServer::~DetectServer() {
            config_.release();
            vidSource.release();
            msgQueue.release();
        }

        int DetectServer::start() {
            if (preview_ and !preview_added) {
                preview_added = true;
                proc->addProcessor(mrtable::process::DisplayFrame::create(1));
            }
            Mat image;
            if (vidSource == NULL) {
                std::cerr << "No video source specified!" << std::endl;
                return 27;
            }
            result_t aggregate;
            aggregate.frames = 0;
            aggregate.elapsed = 0;
            while (vidSource->getFrame(image)) {
                result_t result = proc->process(image);
                if (preview_) {
                    if (SharedData::get<int>(RESULT_KEY_DISPLAYFRAME_KEYPRESS) == 27) {
                        return 27;
                    }
                }
                aggregate.frames++;
                aggregate.elapsed += result.elapsed;
                aggregate.detected += result.detected;
            }
            std::cerr << aggregate << std::endl;
            if (!preview_) return 27;
            return waitKey(0);
        }

        void DetectServer::setVideoSource(cv::Ptr< mrtable::sources::VideoSource > source) {
            vidSource = source;
        }

        void DetectServer::initServer() {
            TUIO::OscSender* sender;
            if (config_->enable_udp) {
                sender = new TUIO::UdpSender(config_->host.c_str(), config_->udp_port);
                server = new TUIO::TuioServer(sender);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            if (config_->enable_tcp) {
                sender = new TUIO::TcpSender(config_->tcp_port);
                if (server == NULL) {
                    server = new TUIO::TuioServer(sender);
                } else {
                    server->addOscSender(sender);
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            if (config_->enable_web) {
                sender = new TUIO::WebSockSender(config_->web_port);
                if (server == NULL) {
                    server = new TUIO::TuioServer(sender);
                } else {
                    server->addOscSender(sender);
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            if (server == NULL) {
                std::cerr << "Warning: No TUIO server was specified in configuration! Attempting to start default server." << std::endl;
                sender = new TUIO::UdpSender("127.0.0.1", 3333);
                server = new TUIO::TuioServer(sender);
            }
        }

        void DetectServer::initProcessQueue() {
            // Prepare frame processors
            SharedData::put(KEY_TUIO_SERVER, server);

            proc = new ProcessQueue();
            proc->addProcessor(mrtable::process::Grayscale::create());
            proc->addProcessor(mrtable::process::Otsu::create());
            //proc->addProcessor(mrtable::process::OtsuCalc::create());
            //proc->addProcessor(mrtable::process::Canny::create());
            proc->addProcessor(mrtable::process::Aruco::create());
            proc->addProcessor(mrtable::process::ArucoCompute::create());
            proc->addProcessor(mrtable::process::Contour::create());
        }

        void DetectServer::setPreview(bool preview) {
            preview_ = preview;
        }
    }
}


#endif
