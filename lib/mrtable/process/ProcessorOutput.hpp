#ifndef __PROCESSOROUTPUT_HPP__
#define __PROCESSOROUTPUT_HPP__

#include <map>
#include <opencv2/core/core.hpp>

using namespace std;

namespace mrtable {
    namespace process {
        class ProcessorOutput {
            public:
                ProcessorOutput() {
                }
                
                ~ProcessorOutput() {
                    outputs.clear();
                }

                static Ptr<ProcessorOutput> create() {
                    return makePtr<ProcessorOutput>();
                }

                template<typename T> T get(int key) {
                    void* val = outputs[key];
                    T* tPtr = static_cast<T*>(val);
                    return *tPtr;
                }

                void put(int key, void* value) {
                    outputs[key] = value;
                }


            private:
                std::map<int, void*> outputs;
        };
    }
}


#endif
