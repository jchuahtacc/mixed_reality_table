#ifndef __MOTIONVECTORPREVIEWCALLBACK_H__
#define __MOTIONVECTORPREVIEWCALLBACK_H__

#include "../typedefs.h"
#include <memory>
#include "MotionVectorCallback.h"
#include "../components/RaspiOverlayRenderer.h"
#include <opencv2/core.hpp>

namespace rpi_motioncam {
    class MotionVectorPreviewCallback : public MotionVectorCallback {
        public: 
            MotionVectorPreviewCallback(RPIMOTIONCAM_OPTION_S options);
            int buffer_position(int row, int col, int row_offset);
            void post_process();
        protected:
            shared_ptr< RaspiOverlayRenderer > renderer_;
    };
}


#endif /* __MOTIONVECTORPREVIEWCALLBACK_H__ */

