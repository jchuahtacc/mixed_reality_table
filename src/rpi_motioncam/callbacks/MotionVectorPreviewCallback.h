#ifndef __MOTIONVECTORPREVIEWCALLBACK_H__
#define __MOTIONVECTORPREVIEWCALLBACK_H__

#include <memory>
#include "rpi_motioncam/callbacks/MotionVectorCallback.h"
#include "rpi_motioncam/components/RaspiOverlayRenderer.h"
#include <opencv2/core.hpp>

namespace rpi_motioncam {
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

