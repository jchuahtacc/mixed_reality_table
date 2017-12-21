#ifndef __MOTIONVECTORPREVIEWCALLBACK_H__
#define __MOTIONVECTORPREVIEWCALLBACK_H__

#include <memory>
#include "MotionVectorCallback.h"
#include "components/RaspiOverlayRenderer.h"
#include <opencv2/core.hpp>

namespace raspivid {
    class MotionVectorPreviewCallback : public MotionVectorCallback {
        public: 
            MotionVectorPreviewCallback(int width, int height);
            void post_process();
        protected:
            shared_ptr< RaspiOverlayRenderer > renderer_;
    };
}


#endif /* __MOTIONVECTORPREVIEWCALLBACK_H__ */

