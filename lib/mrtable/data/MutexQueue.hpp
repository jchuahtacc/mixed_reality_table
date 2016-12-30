#ifndef __MUTEXQUEUE_HPP__
#define __MUTEXQUEUE_HPP__

#include <boost/thread.hpp>

using namespace std;

namespace mrtable {
    namespace data {
        template <class T> class MutexQueue {
            public:
                MutexQueue<T>() {
                }

                ~MutexQueue<T>() {
                    data.clear();
                }

                bool push(T element) {
                    if (mutex.try_lock()) {
                        data.push_back(element);
                        mutex.unlock();
                        return true;
                    }
                    return false;
                }

                bool pop(T& dest) {
                    if (mutex.try_lock()) {
                        dest = data.front();
                        data.pop_front();
                        mutex.unlock();
                        return true;
                    }
                    return false;
                }

                bool isEmpty() {
                    if (mutex.try_lock()) {
                        bool result = data.empty();
                        mutex.unlock();
                        return result;
                    }
                    return false;
                }

                bool pushAll(vector<T> *elements) {
                    if (mutex.try_lock()) {
                        typename vector<T>::iterator it = elements->begin();
                        for (; it < elements->end(); it++) {
                            data.push_back(*it);
                        }
                        mutex.unlock();
                        return true;
                    }
                    return false;
                }

                bool popAll(vector<T> *dest) {
                    if (mutex.try_lock()) {
                        for (typename std::deque<T>::iterator it; it < data.end; it++) {
                            dest->push_back(*it);
                        }
                        mutex.unlock();
                        return true;
                    }
                    return false;
                }
            private:
                std::deque<T> data;
                boost::mutex mutex;
        };
    }
}

#endif
