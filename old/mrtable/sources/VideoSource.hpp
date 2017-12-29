#ifndef __VIDEOSOURCE_HPP__
#define __VIDEOSOURCE_HPP__

#include <opencv2/core/core.hpp>
#include "Source.hpp"
#include <iostream>
#include <string>

namespace mrtable {
    namespace sources {
        class VideoSource : public Source {
            public:
                int fps = 0;

                VideoCapture vid;

                VideoSource(string filename) {
                    opened = vid.open(filename);    
                    if (opened) {
                        this->filename = filename;
                        videofile = true;
                    }
                    getProps();
                }

                static Ptr< VideoSource > create(string filename) {
                    return makePtr< VideoSource >(filename);
                }

                static Ptr< VideoSource > create(int camId) {
                    return makePtr< VideoSource >(camId);
                }

                static Ptr< VideoSource > create() {
                    return makePtr< VideoSource >(0);
                }

                VideoSource(int camId) {
                    opened = vid.open(camId);
                    this->camId = camId;
                    getProps();
                }
                
                bool getFrame(Mat &image) {
                    if (vid.grab()) {
                        vid.retrieve(image);
                        // std::cout << "DisplayFrame::getFrame() " << image.cols "x"  << image.rows << endl;
                        return true;
                    }
                    return false;
                }

                bool reset() {
                    if (videofile) {
                        vid.set(CV_CAP_PROP_POS_FRAMES, 0);
                        return true;
                    }
                    return false;
                }

                bool isFile() {
                    return videofile;
                }

                string getSource() {
                    if (videofile) {
                        return filename;
                    } else {
                        if (camId >= 0) {
                            std::ostringstream ss;
                            ss << "/dev/video" << camId;
                            return ss.str();
                        }
                    }
                    return "";
                }

                friend basic_ostream<char>& operator<<(basic_ostream<char>& outs, VideoSource vid) {
                    outs << "VideoSource from " << vid.getSource() << " ";
                    outs << vid.width << "x" << vid.height << " at " << vid.fps << " fps. Current frame: " << vid.vid.get(CV_CAP_PROP_POS_FRAMES);
                    return outs;
                }

            private:
                bool opened = false;
                bool videofile = false;
                string filename = "";
                int camId = -1;

                void getProps() {
                    width = vid.get(CV_CAP_PROP_FRAME_WIDTH);
                    height = vid.get(CV_CAP_PROP_FRAME_HEIGHT);
                    fps = vid.get(CV_CAP_PROP_FPS);
                    waitTime = 1000 / fps;
                }
        };
    }
}

#endif
