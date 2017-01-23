#ifndef __CONTOURCOMPUTE_HPP__
#define __CONTOURCOMPUTE_HPP__

#include "Touch.h"
#include "FrameProcessor.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <bitset>
#include <iostream>

using namespace mrtable::data;
using namespace mrtable::process;
using namespace cv;
using namespace cv::aruco;
using namespace std;

namespace mrtable {
    namespace process {
        class ContourCompute : public FrameProcessor {
            public: 
                ContourCompute() {
                    skippableFrames = ServerConfig::skippableFrames + 1;
                    movementThresholdPixels = (int)(ServerConfig::movementThreshold * ServerConfig::cameraHeight);;
                    params = ServerConfig::contourParameters;

                    // temporary values
                    minPoints = params->minPointRatio * ServerConfig::cameraWidth;
                    maxPoints = params->maxPointRatio * ServerConfig::cameraWidth;
                    minWidth = params->minWidthRatio * ServerConfig::cameraWidth;
                    maxWidth = params->maxWidthRatio * ServerConfig::cameraWidth;

                    err = "No errors";
                    processor = "Contour Compute";
                }

                ~ContourCompute() {
                    params.release();
                }

                

                vector< Rect > getPotentialTouches(vector< vector<Point> >& contours) {
                    vector< Rect > potentials;
                    for (vector< vector<Point> >::iterator contour = contours.begin(); contour < contours.end(); contour++) {
                        if (contour->size() > minPoints && contour->size() < maxPoints) {
                            Rect r = boundingRect(*contour);
                            if (r.width > minWidth && r.width < maxWidth) {
                                potentials.push_back(r);
                            }
                        }
                    }
                    return potentials;
                }

                void cullPotentials(vector< Touch >& existingTouches, vector< Rect >& potentials, Ptr< SharedData >& data) {
                    for (vector< Touch >::iterator touch = existingTouches.begin(); touch < existingTouches.end(); touch++) {
                        for (vector< Rect >::iterator potential = potentials.begin(); potential < potentials.end(); potential++) {
                            // If there is an overlap, update the existing touch and remove newly scanned contour from list of potential new touches
                            if ((touch->bounds & *potential).area() > 0) {
                                bool changed = touch->calculate(*potential, movementThresholdPixels);
                                touch->deathCounter = -1;
                                if (changed) {
                                    Point2f pos = DetectBounds::getScreenPosition(touch->pos);
                                    data->server->updateTuioCursor(touch->tCur, pos.x, pos.y);
                                }
                                potential = potentials.erase(potential);
                                potential--;
                            }
                        }
                        // Check to see if touch has expired 
                        touch->deathCounter++;
                        if (touch->deathCounter >= skippableFrames) {
                            data->server->removeTuioCursor(touch->tCur);
                            touch = existingTouches.erase(touch);
                            touch--;
                        }
                    }
                }

                void addPotentials(vector< Touch >& existingTouches, vector< Rect >& potentials, Ptr< SharedData >& data) {
                    for (vector< Rect >::iterator potential = potentials.begin(); potential < potentials.end(); potential++) {
                        Touch t = Touch();
                        t.calculate(*potential, movementThresholdPixels);
                        t.tCur = data->server->addTuioCursor(t.pos.x, t.pos.y);
                        existingTouches.push_back(t); 
                    }
                }

                void processRegion(string region, Ptr< SharedData >& data) {
                    vector< Rect > potentials = getPotentialTouches(*(data->contours[region]));
                    if (data->touches.count(region) == 0) {
                        data->touches[region] = makePtr< vector< Touch > >();
                    }
                    cullPotentials(*(data->touches[region]), potentials, data);
                    addPotentials(*(data->touches[region]), potentials, data);
                }

                void processGlobal(Ptr< SharedData >& data) {
                    vector< Rect > potentials = getPotentialTouches(data->globalContours);
                    cullPotentials(data->globalTouches, potentials, data);
                    addPotentials(data->globalTouches, potentials, data);
                }

                bool process(Mat& image, Ptr< SharedData >& data, result_t& result) {
                    if (data->contours.size() <= 0) {
                        processGlobal(data);
                    }

                    // Iterate through contour sets
                    typedef map< string, Ptr< vector< vector<Point> > > >::iterator it_type;
                    for ( it_type it = data->contours.begin(); it != data->contours.end(); ++it) {
                        // Get touch region from image and scan for contours to create list of potential new touches
                        processRegion(it->first, data);

                    }
                    return true;
                }

                static Ptr<FrameProcessor> create() {
                    return makePtr<ContourCompute>().staticCast<FrameProcessor>();
                }

            private:
                int skippableFrames = 4;
                int movementThresholdPixels = 5;
                int minPoints = 100;
                int maxPoints = 300;
                int minWidth = 50;
                int maxWidth = 100;
                Ptr< mrtable::config::ContourParams > params;
        };
    }
}

#endif
