#ifndef __MOTIONVECTORPREVIEWCALLBACK_H__
#define __MOTIONVECTORPREVIEWCALLBACK_H__

#include <memory>
#include "rpi_motioncam/MotionVectorCallback.h"
#include "raspivid/components/RaspiOverlayRenderer.h"
#include <opencv2/core.hpp>

namespace rpi_motioncam {
    using namespace raspivid;
    class MotionVectorPreviewCallback : public MotionVectorCallback {
        public: 
            MotionVectorPreviewCallback(int width, int height);
            int buffer_position(int row, int col, int row_offset);
            void post_process();
        protected:
            shared_ptr< RaspiOverlayRenderer > renderer_;
    };
}


#endif /* __MOTIONVECTORPREVIEWCALLBACK_H__ */

