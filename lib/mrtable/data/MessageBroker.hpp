#ifndef __MESSAGEBROKER_HPP__
#define __MESSAGEBROKER_HPP__

#include <map>
#include <opencv2/core/core.hpp>
#include <sstream>
#include <tuio/TuioTime.h>
#include "Message.hpp"
#include "../server/ServerConnector.h"

using namespace std;
using namespace cv;

namespace mrtable {
    namespace data {
        class MessageBroker {
            public:
                static void create();
                static void destroy();
                static void put(int , string , TUIO::TuioTime );
                static void respond(int, bool, string);
                static void bind(int , Ptr< vector< Message > > );
                static void flush(Ptr< mrtable::data::MutexQueue<string> >);

            private:
                MessageBroker();
                ~MessageBroker();
                static std::map<int , Ptr< vector< Message > > > msgVectors;
                static std::vector< string > responses;
        };

        std::map<int, Ptr< vector< Message > > > mrtable::data::MessageBroker::msgVectors = std::map<int, Ptr< vector< Message > > >();
        std::vector<string> mrtable::data::MessageBroker::responses = std::vector< string >();

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
            msgVector->push_back( Message(cmdCode, paramString, time) );
        }

        void mrtable::data::MessageBroker::respond(int cmdCode, bool ok, string message) {
            stringstream ss;
            ss << cmdCode << " " << (ok ? "OK" : "ERROR") << " " << message << endl;
            server::ServerConnector::server_->broadcast(ss.str());
            std::cerr << "Sending " << ss.str() << endl;
        }

        void MessageBroker::bind(int cmdCode, Ptr< vector< Message > > msgVector) {
            mrtable::data::MessageBroker::msgVectors[cmdCode] = msgVector;
        }
    }
}


#endif
