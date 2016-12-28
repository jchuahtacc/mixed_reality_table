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

                template<typename T> T* getPtr(int key) {
                    void* val = outputs[key];
                    if (val == NULL) {
                        throw std::invalid_argument("ProcessorOutput is missing key");
                    }
                    return static_cast<T*>(val);
                }


                template<typename T> T get(int key) {
                    T* tPtr = getPtr<T>(key);
                    return *tPtr;
                }

                void put(int key, void* value) {
                    outputs[key] = value;
                }

                bool has(int key) {
                    return outputs.count(key) > 0;
                }

                void erase(int key) {
                    std::map<int, void*>::iterator it = outputs.find(key);
                    if (it != outputs.end()) {
                        outputs.erase(outputs.find(key));
                    }
                }


            private:
                std::map<int, void*> outputs;
        };
    }
}


#endif
