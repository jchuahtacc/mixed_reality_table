#ifndef __MESSAGEBROKER_HPP__
#define __MESSAGEBROKER_HPP__

#include <map>
#include <opencv2/core/core.hpp>
#include <sstream>
#include <tuio/TuioTime.h>
#include "Message.hpp"

using namespace std;
using namespace cv;

namespace mrtable {
    namespace data {
        class MessageBroker {
            public:
                static void create();
                static void destroy();
                static void put(int , string , TUIO::TuioTime );
                static void bind(int , Ptr< vector< Message > > );

            private:
                MessageBroker();
                ~MessageBroker();
                static std::map<int , Ptr< vector< Message > > > msgVectors;
        };

        std::map<int, Ptr< vector< Message > > > mrtable::data::MessageBroker::msgVectors = std::map<int, Ptr< vector< Message > > >();

        void mrtable::data::MessageBroker::destroy() {
            for (std::map<int, Ptr< vector< Message > > >::iterator it = msgVectors.begin(); it != msgVectors.end(); it++) {
                it->second.release();
            }
            mrtable::data::MessageBroker::msgVectors.clear();
        }


        void mrtable::data::MessageBroker::put(int cmdCode, string paramString, TUIO::TuioTime time) {
            Ptr< vector< Message > > msgVector = msgVectors[cmdCode];
            if (msgVector.empty()) {
                std::cerr << "MessageBroker exception - no msgVector bound to " << cmdCode << std::endl;
                return;
            }
            mrtable::data::MessageBroker::msgVectors[cmdCode]->push_back( Message(cmdCode, paramString, time) );
        }

        void MessageBroker::bind(int cmdCode, Ptr< vector< Message > > msgVector) {
            mrtable::data::MessageBroker::msgVectors[cmdCode] = msgVector;
        }
    }
}


#endif
