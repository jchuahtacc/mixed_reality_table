#ifndef __CONTOURCOMPUTE_HPP__
#define __CONTOURCOMPUTE_HPP__

#include "Touch.hpp"
#include "FrameProcessor.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <bitset>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

using namespace mrtable::data;
using namespace mrtable::process;
using namespace cv;
using namespace cv::aruco;
using namespace std;

namespace mrtable {
    namespace process {
        class ContourCompute : public FrameProcessor {
            public: 
                typedef vector< string > token_list;
                vector< Touch >* allTouches;
                std::map< string, vector< Touch >* > touches;
                std::map< string, Rect_<float>* > regions;
                Ptr< vector< Message > > msgVector;

                ContourCompute() {
                    msgVector = makePtr< vector< Message > >();
                    allTouches = new vector< Touch >();
                    MessageBroker::bind(CMD_PUT_REGION, msgVector);
                    MessageBroker::bind(CMD_UPDATE_REGION, msgVector);
                    MessageBroker::bind(CMD_DELETE_REGION, msgVector);
                    skippableFrames = ServerConfig::skippableFrames + 1;
                    movementThreshold = ServerConfig::movementThreshold;
                    params = ServerConfig::contourParameters;

                    // temporary values
                    minPoints = params->minPointRatio * ServerConfig::cameraWidth;
                    maxPoints = params->maxPointRatio * ServerConfig::cameraWidth;
                    minWidth = params->minWidthRatio * ServerConfig::cameraWidth;
                    maxWidth = params->maxWidthRatio * ServerConfig::cameraWidth;

                    contours = SharedData::getPtr< vector< vector<Point> > >(RESULT_KEY_CONTOUR_CONTOURS);
                    hierarchy = SharedData::getPtr< vector<Vec4i> >(RESULT_KEY_CONTOUR_HIERARCHY);

                    SharedData::put(RESULT_KEY_CONTOUR_TOUCHES, &touches);
                    err = "No errors";
                    processor = "Contour Compute";
                }

                ~ContourCompute() {
                    SharedData::erase(RESULT_KEY_CONTOUR_TOUCHES);
                    msgVector.release();
                    params.release();
                }

                void putRegion(Message msg) {
                    token_list tokens;
                    boost::split(tokens, msg.params, boost::is_any_of(" "), boost::token_compress_on);
                    if (tokens.size() == 5) {
                        try {
                            string region = tokens[0];
                            boost::trim(region);
                            if (regions.count(region)) {
                                delete regions[region];
                            }
                            if (touches.count(region)) {
                                delete touches[region];
                            }
                            Rect_<float>* r = new Rect_<float>();
                            r->x = stof(tokens[1]);
                            r->y = stof(tokens[2]);
                            r->width = stof(tokens[3]);
                            r->height = stof(tokens[4]);
                            regions[region] = r;
                            touches[region] = new vector< Touch >();
                        } catch (...) {
                            std::cerr << "CMD_PUT_REGION error parsing rectangle dimensions " << msg.params << std::endl;
                        }
                    } else {
                        std::cerr << "CMD_PUT_REGION expects 5 parameters. Received: " << msg.params << std::endl;
                    }
                }

                void updateRegion(Message msg) {
                    putRegion(msg);
                }

                void deleteRegion(Message msg) {
                    boost::trim(msg.params);
                    if (regions.count(msg.params) > 0) {
                        delete regions[msg.params];
                        regions.erase(msg.params);
                    }
                    if (touches.count(msg.params) > 0) {
                        delete touches[msg.params];
                        touches.erase(msg.params);
                    }
                }

                bool process(Mat& image, result_t& result) {
                    for ( vector< Message >::iterator msg = msgVector->begin(); msg < msgVector->end(); msg++) {
                        switch (msg->cmdCode) {
                            case CMD_PUT_REGION: putRegion(*msg); break;
                            case CMD_UPDATE_REGION: updateRegion(*msg); break;
                            case CMD_DELETE_REGION: deleteRegion(*msg); break;
                        }
                    }
                    msgVector->clear();

                    for ( std::map<string, Rect_<float>* >::iterator it = regions.begin(); it != regions.end(); ++it) {
                        // std::cout << "Region " << it->first <<  " x: " << it->second->x << " y: " << it->second->y << " width: " << it->second->width << " height: " << it->second->height << std::endl;
                    }
                    return true;
                }

                static Ptr<FrameProcessor> create() {
                    return makePtr<ContourCompute>().staticCast<FrameProcessor>();
                }

            private:
                int skippableFrames = 4;
                int movementThreshold = 5;
                int minPoints = 100;
                int maxPoints = 300;
                int minWidth = 50;
                int maxWidth = 100;
                vector< vector<Point> > *contours;
                vector<Vec4i> *hierarchy;
                Ptr< mrtable::config::ContourParams > params;
        };
    }
}

#endif
