#ifndef __DETECTBOUNDS_H__
#define __DETECTBOUNDS_H__

#include "ServerConfig.h"
#include <opencv2/aruco.hpp>
#include <opencv2/core/core.hpp>
#include <cmath>

using namespace std;
using namespace cv;

namespace mrtable {
    namespace config {
        class DetectBounds {
            public:
                static bool verifyMarkerPlacement(Mat&, cv::Ptr<cv::aruco::Dictionary>, cv::Ptr<cv::aruco::DetectorParameters>);
                static bool calculateRoi();
                static bool read(string);
                static bool write(string);
                static int rot;
                static int x;
                static int y;
                static int width;
                static int height;
                static int sections;
                static int sectionNum;
                static int leftOverflow;
                static void dump(basic_ostream<char>&);
                static Point2f getScreenPosition(Point2f pos);
                static void setDefaults();
                static int baseline;
                static int baseheight;
            private:
                static vector< int > ids;
                static vector< vector<Point2f> > corners;
                static vector<int> orientations;
                static bool sortMarkers();
                static int getOrientation(vector< Point2f >);
                static int getRot();
                template <class T> static vector<T> makeVector(int);
                DetectBounds();
                ~DetectBounds();
        };
    }
}

#endif
