#ifndef __SHAREDDATA_HPP__
#define __SHAREDDATA_HPP__

#include <map>
#include <opencv2/core/core.hpp>
#include <sstream>

using namespace std;
using namespace cv;

namespace mrtable {
    namespace data {
        class SharedData {
            public:
                static void create();
                static void destroy();
                template<typename T> static T* getPtr(int key); 
                template<typename T> static T get(int key);
                static void put(int key, void* value);
                static bool has(int key);
                static void erase(int key);

            private:
                SharedData();
                ~SharedData();
                static std::map<int, void* > dataMap;
        };

        std::map<int, void*> mrtable::data::SharedData::dataMap = std::map<int, void*>();

        void mrtable::data::SharedData::destroy() {
            mrtable::data::SharedData::dataMap.clear();
        }

        template<typename T> T* mrtable::data::SharedData::getPtr(int key) {
            void* val = mrtable::data::SharedData::dataMap[key];
            return static_cast<T*>(val);
        }


        template<typename T> T mrtable::data::SharedData::get(int key) {
            if (!has(key)) {
                stringstream ss;
                ss << "SharedData invalid_argument exception - invalid key: " << key;
                throw std::invalid_argument(ss.str());
            }
            T* tPtr = mrtable::data::SharedData::getPtr<T>(key);
            return *tPtr;
        }

        void mrtable::data::SharedData::put(int key, void* value) {
            mrtable::data::SharedData::dataMap[key] = value;
        }

        bool mrtable::data::SharedData::has(int key) {
            return mrtable::data::SharedData::dataMap.count(key) > 0;
        }

        void mrtable::data::SharedData::erase(int key) {
            std::map<int, void*>::iterator it = mrtable::data::SharedData::dataMap.find(key);
            if (it != mrtable::data::SharedData::dataMap.end()) {
                mrtable::data::SharedData::dataMap.erase(mrtable::data::SharedData::dataMap.find(key));
            }
        }

    }
}


#endif
