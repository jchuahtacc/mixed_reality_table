#include <opencv2/core/core.hpp>
#include <iostream>
#include <string>
#include "ContourParams.h"

using namespace std;
using namespace cv;

namespace mrtable {
    namespace config {
        shared_ptr<ContourParams> ContourParams::create(string filename) {
            return shared_ptr<ContourParams>(new ContourParams(filename));
        }

        bool ContourParams::read(string filename) {
            ContourParams* params = this;
            FileStorage fs(filename, FileStorage::READ);
            if (!fs.isOpened())
                return false;
            fs["minPointRatio"] >> params->minPointRatio;
            fs["maxPointRatio"] >> params->maxPointRatio;
            fs["minWidthRatio"] >> params->minWidthRatio;
            fs["maxWidthRatio"] >> params->maxWidthRatio;
            return true;
        }

        bool ContourParams::write(string filename) {
            ContourParams* params = this;
            FileStorage fs(filename, FileStorage::WRITE);
            if (!fs.isOpened())
                return false;
            fs << "minPointRatio" << params->minPointRatio;
            fs << "maxPointRatio" << params->maxPointRatio;
            fs << "minWidthRatio" << params->minWidthRatio;
            fs << "maxWidthRatio" << params->maxWidthRatio;
            fs.release();
            return true;
        }

        bool ContourParams::parse(cv::String& paramString) {
            ContourParams *params = this;
            istringstream ss(paramString);
            string token;
            while (std::getline(ss, token, ',')) {
                istringstream pair(token);
                string key, value;
                if (std::getline(pair, key, '=') && std::getline(pair, value)) {
                    transform(key.begin(), key.end(), key.begin(), ::tolower);
                    transform(value.begin(), value.end(), value.begin(), ::tolower);
                    if (key.compare("minpointratio") == 0) {
                        params->minPointRatio = stof(value);
                    }
                    if (key.compare("maxpointratio") == 0) {
                        params->maxPointRatio = stof(value);
                    }
                    if (key.compare("minradiusratio") == 0) {
                        params->minWidthRatio = stof(value);
                    }
                    if (key.compare("maxradiusratio") == 0) {
                        params->maxWidthRatio = stof(value);
                    }
                }                
            }
            return true;
        }

        basic_ostream<char>& operator<<(basic_ostream<char>& outs, ContourParams params) {
            outs << "minPointRatio" << params.minPointRatio << endl;
            outs << "maxPointRatio" << params.maxPointRatio << endl;
            outs << "minWidthRatio" << params.minWidthRatio << endl;
            outs << "maxWidthRatio" << params.maxWidthRatio << endl;
            return outs; 
        }

        basic_ostream<char>& operator<<(basic_ostream<char>& outs, shared_ptr<ContourParams> params) {
            outs << *(params.get());
            return outs;
        }

        ContourParams::ContourParams() {
        }

        ContourParams::ContourParams(string filename) {
            if (!filename.empty()) {
                this->read(filename);
            }
        }

        ContourParams::~ContourParams() {
        }
    }
}
