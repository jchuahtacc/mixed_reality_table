#ifndef __CONTOUR_HPP__
#define __CONTOUR_HPP__

#include "Marker.h"
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

                Contour() {
                    err = "No errors";
                    processor = "Contour";
                }

                ~Contour() {
                }

                void putRegion(Message msg, Ptr< SharedData >& data) {
                    token_list tokens;
                    boost::split(tokens, msg.params, boost::is_any_of(" "), boost::token_compress_on);
                    if (tokens.size() == 5) {
                        try {
                            string region = tokens[0];
                            boost::trim(region);
                            if (data->regions.count(region)) {
                                data->regions[region].release();
                            }
                            data->regions[region] = makePtr< Rect_<float> >();
                            Ptr < Rect_<float> > r = data->regions[region];
                            r->x = stof(tokens[1]);
                            r->y = stof(tokens[2]);
                            r->width = stof(tokens[3]);
                            r->height = stof(tokens[4]);
                            data->contours[region] = makePtr< vector< vector< Point > > >();
                            data->hierarchies[region] = makePtr< vector< Vec4i > >();
                            MessageBroker::respond(msg.cmdCode, true, msg.params);
                        } catch (...) {
                            MessageBroker::respond(msg.cmdCode, false, "CMD_PUT_REGION error parsing rectangle dimensions");
                        }
                    } else {
                        MessageBroker::respond(msg.cmdCode, false, "CMD_PUT_REGION expects 5 parameters");
                    }
                }

                void updateRegion(Message msg, Ptr< SharedData >& data) {
                    putRegion(msg, data);
                }

                void deleteRegion(Message msg, Ptr< SharedData >& data) {
                    boost::trim(msg.params);
                    if (data->regions.count(msg.params) > 0) {
                        data->regions[msg.params].release();
                        data->contours[msg.params].release();
                        data->hierarchies[msg.params].release();
                        data->regions.erase(msg.params);
                        data->contours.erase(msg.params);
                        data->hierarchies.erase(msg.params);
                        MessageBroker::respond(msg.cmdCode, true, msg.params);
                    } else {
                        MessageBroker::respond(msg.cmdCode, false, msg.params);
                    }
                }

                bool process(Mat& image, Ptr< SharedData >& data, result_t& result) {
                    for ( vector< Message >::iterator msg = data->regionMessages->begin(); msg < data->regionMessages->end(); msg++) {
                        switch (msg->cmdCode) {
                            case CMD_PUT_REGION: putRegion(*msg, data); break;
                            case CMD_UPDATE_REGION: updateRegion(*msg, data); break;
                            case CMD_DELETE_REGION: deleteRegion(*msg, data); break;
                        }
                    }
                    data->regionMessages->clear();
                    if (data->regions.size() <= 0) {
                        findContours(image, data->globalContours, data->globalHierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE);
                    } else {
                        typedef map< string, Ptr< Rect_<float> > >::iterator it_type;
                        for (it_type it = data->regions.begin(); it != data->regions.end(); it++) {
                            string region = it->first;
                            Rect_<float> rect = *(it->second);

                            Mat subImage = image(Rect(rect.x * ServerConfig::cameraWidth, rect.y * ServerConfig::cameraHeight, rect.width * ServerConfig::cameraHeight, rect.height * ServerConfig::cameraHeight));
                            findContours(subImage, *(data->contours[region]), *(data->hierarchies[region]), RETR_LIST, CHAIN_APPROX_SIMPLE, Point(rect.x, rect.y));
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
