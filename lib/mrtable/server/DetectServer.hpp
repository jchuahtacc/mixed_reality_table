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
                DetectServer(cv::Ptr< mrtable::config::ServerConfig > config, cv::Ptr< mrtable::data::MutexQueue<string> > msgQueue);
                ~DetectServer();
                bool start();
                void setVideoSource(cv::Ptr< mrtable::sources::VideoSource > videoSource);
                void setPreview(bool preview);
                int processFrame(Mat image);

            private:
                void initTuioServer();
                void initProcessQueue();
                Ptr< mrtable::config::ServerConfig > config_;
                Ptr< mrtable::sources::VideoSource > vidSource;
                ProcessQueue* proc = NULL;
                TUIO::TuioServer* server = NULL;
                Ptr< MutexQueue<string> > msgQueue_;
                bool preview_ = false;
                bool preview_added = false;
                int* keyPress = NULL;
                vector< string > messages;

        };

        DetectServer::DetectServer(Ptr< mrtable::config::ServerConfig > config, cv::Ptr< mrtable::data::MutexQueue<string> > msgQueue) : config_(config), msgQueue_(msgQueue) {
            SharedData::put(KEY_CONFIG, config_);
            msgQueue = new MutexQueue<string>();
            SharedData::put(KEY_MSG_QUEUE, msgQueue);
            initTuioServer();
            initProcessQueue();
        }

        DetectServer::~DetectServer() {
            config_.release();
            vidSource.release();
            msgQueue_.release();
        }

        bool DetectServer::start() {
            if (preview_ and !preview_added) {
                preview_added = true;
                proc->addProcessor(mrtable::process::DisplayFrame::create(1));
                keyPress = SharedData::getPtr<int>(RESULT_KEY_DISPLAYFRAME_KEYPRESS);
            }
            bool keepRunning = true;
            Mat image;
            if (vidSource.empty()) {
                std::cerr << "No video source specified!" << std::endl;
                return false;
            }
            result_t aggregate;
            aggregate.frames = 0;
            aggregate.elapsed = 0;
            aggregate.detected = 0;

            while (keepRunning) {
                // Get messages from message Queue
                if (msgQueue_->popAll(&messages)) {
                    // got messages
                }

                bool gotFrame = vidSource->getFrame(image);
                if (!gotFrame) {
                    if (vidSource->isFile()) {
                        if (preview_) {
                            std::cout << "Video playback finished. Press any key to restart, or ESC key to exit..." << std::endl;
                            int key = waitKey(0);
                            if (key == CMD_ESCAPE_KEY) {
                                keepRunning = false;
                            }
                        }

                        // Loop video
                        vidSource->reset();

                    } else {
                        keepRunning = false;
                        std::cerr << "Unable to get video frame from " << vidSource->getSource() << std::endl;
                    }
                } else {
                    result_t result = proc->process(image);
                    aggregate.frames++;
                    aggregate.elapsed += result.elapsed;
                    aggregate.detected += result.detected;
                    if (aggregate.frames % 30 == 0) {
                        std::cerr << aggregate << std::endl;
                    }

                    if (keyPress != NULL) {
                        // Check for escape key during preview playback
                        if (*keyPress == CMD_ESCAPE_KEY) {
                            keepRunning = false;
                        }
                    }
                }
            }

            std::cerr << aggregate << std::endl;

            return true;
        }

        void DetectServer::setVideoSource(cv::Ptr< mrtable::sources::VideoSource > source) {
            vidSource = source;
        }

        void DetectServer::initTuioServer() {
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
