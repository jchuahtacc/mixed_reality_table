#ifndef __CONTOUR_HPP__
#define __CONTOUR_HPP__

#include "Marker.hpp"
#include "FrameProcessor.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <bitset>
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
        class Contour : public FrameProcessor {
            public: 
                typedef vector< string > token_list;
                map< string, vector<Vec4i>* > hierarchies;
                map< string, Rect_<float>* >  regions;
                Ptr< vector< Message > > msgVector;
                map< string, vector< vector<Point> >* > contours;
                vector< vector<Point> > globalContours;
                vector< Vec4i > globalHierarchy;

                Contour() {
                    msgVector = makePtr< vector< Message > >();

                    MessageBroker::bind(CMD_PUT_REGION, msgVector);
                    MessageBroker::bind(CMD_UPDATE_REGION, msgVector);
                    MessageBroker::bind(CMD_DELETE_REGION, msgVector);
                    SharedData::put(RESULT_KEY_CONTOUR_CONTOURS, &contours);
                    SharedData::put(RESULT_KEY_CONTOUR_HIERARCHY, &hierarchies);
                    SharedData::put(RESULT_KEY_CONTOUR_REGIONS, &regions);
                    SharedData::put(RESULT_KEY_CONTOUR_GLOBAL_CONTOURS, &globalContours);
                    SharedData::put(RESULT_KEY_CONTOUR_GLOBAL_HIERARCHY, &globalHierarchy);
                    err = "No errors";
                    processor = "Contour";
                }

                ~Contour() {
                    SharedData::erase(RESULT_KEY_CONTOUR_CONTOURS);
                    SharedData::erase(RESULT_KEY_CONTOUR_HIERARCHY);
                    SharedData::erase(RESULT_KEY_CONTOUR_REGIONS);
                    SharedData::erase(RESULT_KEY_CONTOUR_GLOBAL_CONTOURS);
                    msgVector.release();
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
                            Rect_<float>* r = new Rect_<float>();
                            r->x = stof(tokens[1]);
                            r->y = stof(tokens[2]);
                            r->width = stof(tokens[3]);
                            r->height = stof(tokens[4]);
                            regions[region] = r;
                            contours[region] = new vector< vector<Point> >();
                            hierarchies[region] = new vector< Vec4i >();
                            MessageBroker::respond(msg.cmdCode, true, msg.params);
                        } catch (...) {
                            MessageBroker::respond(msg.cmdCode, false, "CMD_PUT_REGION error parsing rectangle dimensions");
                        }
                    } else {
                        MessageBroker::respond(msg.cmdCode, false, "CMD_PUT_REGION expects 5 parameters");
                    }
                }

                void updateRegion(Message msg) {
                    putRegion(msg);
                }

                void deleteRegion(Message msg) {
                    boost::trim(msg.params);
                    if (regions.count(msg.params) > 0) {
                        delete regions[msg.params];
                        delete contours[msg.params];
                        delete hierarchies[msg.params];
                        regions.erase(msg.params);
                        contours.erase(msg.params);
                        hierarchies.erase(msg.params);
                        MessageBroker::respond(msg.cmdCode, true, msg.params);
                    } else {
                        MessageBroker::respond(msg.cmdCode, false, msg.params);
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
                    if (regions.size() <= 0) {
                        findContours(image, globalContours, globalHierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE);
                    } else {
                        typedef map< string, Rect_<float>* >::iterator it_type;
                        for (it_type it = regions.begin(); it != regions.end(); it++) {
                            string region = it->first;
                            Rect_<float> rect = *(it->second);

                            Mat subImage = image(Rect(rect.x * ServerConfig::cameraWidth, rect.y * ServerConfig::cameraHeight, rect.width * ServerConfig::cameraHeight, rect.height * ServerConfig::cameraHeight));
                            findContours(subImage, *(contours[region]), *(hierarchies[region]), RETR_LIST, CHAIN_APPROX_SIMPLE, Point(rect.x, rect.y));
                        }
                    }
                    return true;
                }

                static Ptr<FrameProcessor> create() {
                    return makePtr<Contour>().staticCast<FrameProcessor>();
                }

        };
    }
}

#endif
