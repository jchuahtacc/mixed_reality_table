#ifndef __DETECTBOUNDS_HPP__
#define __DETECTBOUNDS_HPP__

#include <opencv2/aruco.hpp>
#include <opencv2/core/core.hpp>

namespace mrtable {
    namespace server {
        class DetectBounds {
            public:
                static bool verifyMarkerPlacement(Mat&);
                static bool calculateRoi();
                static int orientation;
                static int x;
                static int y;
                static int width;
                static int height;
                static int sections;
                static int sectionNum;
            private:
                static vector< int > ids;
                static vector< vector<Point2f> > corners;
                static vector<int> orientations;
                static bool sortMarkers();
                static int getOrientation(vector< Point2f >);
                static int getMarkerOrientation();
                template <class T> static vector<T> makeVector(int);
                DetectBounds();
                ~DetectBounds();
        };


        vector< int > DetectBounds::ids = makeVector<int>(4);
        vector< vector<Point2f> > DetectBounds::corners = makeVector< vector<Point2f> >(4);
        vector< int > DetectBounds::orientations = makeVector<int>(4);
        int DetectBounds::x = 0;
        int DetectBounds::y = 0;
        int DetectBounds::width = 0;
        int DetectBounds::height = 0;
        int DetectBounds::orientation = -1;
        int DetectBounds::sections = 0;
        int DetectBounds::sectionNum = 0;

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

        int DetectBounds::getMarkerOrientation() {
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

        bool DetectBounds::verifyMarkerPlacement(Mat& image) {
            vector< vector< Point2f > > rejected;
            aruco::detectMarkers(image, mrtable::config::ServerConfig::dictionary, corners, ids, mrtable::config::ServerConfig::detectorParameters, rejected);

            // must have exactly 4 markers
            if (ids.size() != 4) {
                return false;
            }

            if (!sortMarkers()) {
                return false;
            }

            // highest marker must be at least 3
            if (ids[3] < 3) {
                return false;
            }

            // highest marker number must be odd
            if (ids[3] % 2 != 1) {
                return false;
            }

            // highest and lowest marker sum must be equivalent to sum of middle markers
            if (ids[0] + ids[3] != ids[1] + ids[2]) {
                return false;
            }

            // First two markers and second two markers must be adjacent ids
            if (ids[0] + 1 != ids[1]) {
                return false;
            }
            if (ids[2] + 1 != ids[3]) {
                return false;
            }

            orientations.resize(4);
            for (int i = 0; i < 4; i++) {
                orientations[i] = getOrientation(corners[i]);
            }

            // First marker must have same orientation as second marker (or second marker must be 90 degrees clockwise of first)
            if (orientations[0] != orientations[1] && (orientations[0] + 1) % 4 != orientations[1]) {
                return false;
            }

            // 3rd marker must be 90 degrees or 180 degrees clockwise of 2nd marker
            if ((orientations[1] + 1) % 4 != orientations[2] && (orientations[1] + 2) % 4 != orientations[2]) {
                return false;
            }

            // 4th marker must be same orientation as 3rd marker, or 90 degrees clockwise of 3rd marker
            if (orientations[2] != orientations[3] && (orientations[2] + 1) % 4 != orientations[3]) {
                return false;
            }

            // If 2nd and 3rd marker are adjacent numbers, they must be the right hand side of the table
            if (ids[1] + 1 == ids[2] && (orientations[1] + 1) % 4 != orientations[2]) {
                return false;
            }

            return true;
        }

        bool DetectBounds::calculateRoi() {
            orientation = getMarkerOrientation();
            sections = (ids[3] - 1) / 2;
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
            return true;
        }
    }
}

#endif
