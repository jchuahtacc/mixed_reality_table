#ifndef __DETECTBOUNDS_CPP__
#define __DETECTBOUNDS_CPP__

#include "ServerConfig.h"
#include "DetectBounds.h"
#include <opencv2/aruco.hpp>
#include <opencv2/core/core.hpp>
#include <cmath>

using namespace std;
using namespace cv;

namespace mrtable {
    namespace config {
        vector< int > DetectBounds::ids = makeVector<int>(4);
        vector< vector<Point2f> > DetectBounds::corners = makeVector< vector<Point2f> >(4);
        vector< int > DetectBounds::orientations = makeVector<int>(4);

        int DetectBounds::x = 0;
        int DetectBounds::y = 0;
        int DetectBounds::width = 0;
        int DetectBounds::height = 0;
        int DetectBounds::rot = -1;
        int DetectBounds::sections = 0;
        int DetectBounds::sectionNum = 0;
        int DetectBounds::leftOverflow = 0;
        int DetectBounds::baseline = 0;
        int DetectBounds::baseheight = 0;

        bool DetectBounds::read(string filename) {
            setDefaults();
            if (filename.empty()) {
                return false;
            }
            FileStorage fs(filename, FileStorage::READ);
            if (!fs.isOpened()) {
                return false;
            }

            fs["x"] >> x;
            fs["y"] >> y;
            fs["width"] >> width;
            fs["height"] >> height;
            fs["rot"] >> rot;
            fs["sections"] >> sections;
            fs["sectionNum"] >> sectionNum;
            fs["leftOverflow"] >> leftOverflow;
            fs["baseline"] >> baseline;
            fs["baseheight"] >> baseheight;
            return true;
        }

        bool DetectBounds::write(string filename) {
            if (filename.empty()) {
                return false;
            }

            FileStorage fs(filename, FileStorage::WRITE);
            if (!fs.isOpened()) {
                return false;
            }

            fs << "x" << x;
            fs << "y" << y;
            fs << "width" << width;
            fs << "height" << height;
            fs << "rot" << rot;
            fs << "sections" << sections;
            fs << "sectionNum" << sectionNum;
            fs << "leftOverflow" << leftOverflow;
            fs << "baseline" << baseline;
            fs << "baseheight" << baseheight;
            return true;

        }

        void DetectBounds::setDefaults() {
            x = 0;
            y = 0;
            width = ServerConfig::cameraWidth;
            height = ServerConfig::cameraHeight;
            rot = 0;
            sections = 1;
            sectionNum = 0;
            leftOverflow = 0;
            baseline = width;
            baseheight = height;
        }

        Point2f DetectBounds::getScreenPosition(Point2f pos) {
            Point2f result; 

            /*
            if (rot == 0 ) {
                // no transform
            }
            */

            if (rot == 1) {
                pos.x -= width;

                float temp = pos.x;
                pos.x = pos.y;
                pos.y = -temp;
            }
            if (rot == 2) {
                pos.x -= width;
                pos.y -= height;
                pos.x = -pos.x;
                pos.y = -pos.y;
            }

            if (rot == 3) {
                pos.y -= height;
                float temp = pos.x;
                pos.x = -pos.y;
                pos.y = temp;
            }

            pos.x -= leftOverflow;

            result.x = 1.0 / sections * pos.x / (float)baseline + (float)sectionNum / sections;
            result.y = pos.y / (float)baseheight;

            return result;
        }

        template <class T> vector<T> DetectBounds::makeVector(int size) {
            vector<T> vec;
            vec.resize(size);
            return vec;
        }

        int DetectBounds::getOrientation(vector<Point2f> corners) {
            if (corners[0].x < corners[2].x && corners[0].y < corners[2].y) {
                return 0;
            }
            if (corners[0].x > corners[2].x && corners[0].y < corners[2].y) {
                return 1;
            }
            if (corners[0].x > corners[2].x && corners[0].y > corners[2].y) {
                return 2;
            }
            if (corners[0].x < corners[2].x && corners[0].y > corners[2].y) {
                return 3;
            }
            return -1;
        }

        int DetectBounds::getRot() {
            if (corners[0][0].x < corners[2][0].x && corners[0][0].y < corners[2][0].y) {
                return 0;
            }
            if (corners[0][0].x > corners[2][0].x && corners[0][0].y < corners[2][0].y) {
                return 1;
            }
            if (corners[0][0].x > corners[2][0].x && corners[0][0].y > corners[2][0].y) {
                return 2;
            }
            if (corners[0][0].x < corners[2][0].x && corners[0][0].y > corners[2][0].y) {
                return 3;
            }
            return -1;
        }

        bool DetectBounds::sortMarkers() {
            if (ids.size() != 4 || corners.size() != 4) {
                return false;
            }
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4 - i - 1; j++) {
                    if (ids[j] == ids[j + 1]) {
                        // duplicate marker trap
                        return false;
                    }
                    if (ids[j] > ids[j + 1]) {
                        int tempId = ids[j];
                        vector< Point2f > tempCorners = corners[j];
                        ids[j] = ids[j + 1];
                        corners[j] = corners[j + 1];
                        ids[j + 1] = tempId;
                        corners[j + 1] = tempCorners;
                    }
                }
            }
            return true;
        }

        bool DetectBounds::verifyMarkerPlacement(Mat& image, cv::Ptr<cv::aruco::Dictionary> dictionary, cv::Ptr<cv::aruco::DetectorParameters> detectorParameters) {
            vector< vector< Point2f > > rejected;
            aruco::detectMarkers(image, dictionary, corners, ids, detectorParameters, rejected);

            // must have exactly 4 markers
            if (ids.size() != 4) {
                return false;
            }
            //cout << "4 markers detected" << endl;

            if (!sortMarkers()) {
                return false;
            }
            //cout << "Markers sorted, no duplicates" << endl;

            // highest marker must be at least 3
            if (ids[3] < 3) {
                return false;
            }
            //cout << "Highest marker id is at least 3" << endl;

            // Opposite marker id sum must be odd
            if ((ids[3] + ids[0]) % 2 != 1) {
                return false;
            }
            //cout << "Opposite marker id sum is odd" << endl;

            // highest and lowest marker sum must be equivalent to sum of middle markers
            if (ids[0] + ids[3] != ids[1] + ids[2]) {
                return false;
            }
            //cout << "Marker pair sums match" << endl;

            // First two markers and second two markers must be adjacent ids
            if (ids[0] + 1 != ids[1]) {
                return false;
            }
            if (ids[2] + 1 != ids[3]) {
                return false;
            }
            //cout << "Markers are sequential" << endl;

            orientations.resize(4);
            for (int i = 0; i < 4; i++) {
                orientations[i] = getOrientation(corners[i]);
            }

            // First marker must have same orientation as second marker (or second marker must be 90 degrees clockwise of first)
            if (orientations[0] != orientations[1] && (orientations[0] + 1) % 4 != orientations[1]) {
                return false;
            }
            //cout << "First marker pair orientations are correct" << endl;

            // 3rd marker must be 90 degrees or 180 degrees clockwise of 2nd marker
            if ((orientations[1] + 1) % 4 != orientations[2] && (orientations[1] + 2) % 4 != orientations[2]) {
                return false;
            }
            //cout << "Middle marker pair orientations are correct" << endl;

            // 4th marker must be same orientation as 3rd marker, or 90 degrees clockwise of 3rd marker
            if (orientations[2] != orientations[3] && (orientations[2] + 1) % 4 != orientations[3]) {
                return false;
            }
            //cout << "Second marker pair orientations are correct" << endl;

            // If 2nd and 3rd marker are adjacent numbers, they must be the right hand side of the table
            if (ids[1] + 1 == ids[2] && (orientations[1] + 1) % 4 != orientations[2]) {
                return false;
            }
            //cout << "Right hand marker orientations checked for rightbound orientations" << endl;

            return true;
        }

        bool DetectBounds::calculateRoi() {
            rot = getRot();
            sections = (ids[0] + ids[3] - 1) / 2;
            sectionNum = ids[0];
            int minX = corners[0][0].x;
            int maxX = corners[0][0].x;
            int minY = corners[0][0].y;
            int maxY = corners[0][0].y;
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    if (corners[i][j].x < minX) minX = corners[i][j].x;
                    if (corners[i][j].x > maxX) maxX = corners[i][j].x;
                    if (corners[i][j].y < minY) minY = corners[i][j].y;
                    if (corners[i][j].y > maxY) maxY = corners[i][j].y;
                }
            }
            x = minX;
            y = minY;
            width = maxX - minX;
            height = maxY - minY;
            
            bool leftBound = false;
            bool rightBound = false;

            if (ids[0] == 0) {
                leftBound = true;
            }

            if (ids[1] + 1 == ids[2]) {
                rightBound = false;
            }


            int interiorDistance  = 0;
            baseline = 0;
            if (rot == 0 || rot == 2) {
                baseline = abs(corners[0][0].x - corners[2][0].x);
                baseheight = abs(corners[0][0].y - corners[2][0].y);
                leftOverflow = x;
            } else {
                baseline = abs(corners[0][0].y - corners[2][0].y);
                baseheight = abs(corners[0][0].x - corners[2][0].x);
                leftOverflow = y;
            }

            if (rot == 0) {
                leftOverflow = corners[0][0].x - x;
            }
            if (rot == 2) {
                leftOverflow = x + width - corners[0][0].x;
            }

            if (rot == 1) {
                leftOverflow = corners[0][0].y - y;
            }
            if (rot == 3) {
                leftOverflow = y + height - corners[0][0].y;
            }

            return true;
        }

        void DetectBounds::dump(basic_ostream<char>& outs) {
            outs << "*** Detected Bounds ***" << endl;
            outs << "Region of interest - x: " << DetectBounds::x << " y: " << DetectBounds::y << " width: " << DetectBounds::width << " height: " << DetectBounds::height << endl;
            outs << "Section " << DetectBounds::sectionNum << " of " << DetectBounds::sections << " with rot " << DetectBounds::rot << endl;
            outs << "baseline: " << baseline << " baseheight: " << baseheight << " left overflow: " << endl;
            outs << "Section screen origin: " << DetectBounds::getScreenPosition(Point2f(0, 0)) << endl;
        }
    }
}

#endif
