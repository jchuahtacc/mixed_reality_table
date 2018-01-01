#ifndef __MOTIONREGIONPREVIEWCALLBACK_H__
#define __MOTIONREGIONPREVIEWCALLBACK_H__

#include "../typedefs.h"
#include <memory>
#include "MotionVectorCallback.h"
#include "../components/RaspiOverlayRenderer.h"
#include <opencv2/core.hpp>

namespace rpi_motioncam {
    class MotionRegionPreviewCallback : public MotionVectorCallback {
        public: 
            MotionRegionPreviewCallback(RPIMOTIONCAM_OPTION_S options);
            int buffer_position(int row, int col, int row_offset);
            void post_process();
        protected:
            shared_ptr< RaspiOverlayRenderer > renderer_;
    };
}


#endif /* __MOTIONVECTORPREVIEWCALLBACK_H__ */

