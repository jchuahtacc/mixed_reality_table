#ifndef __FRAMEPREVIEW_H__
#define __FRAMEPREVIEW_H__

#include "../typedefs.h"
#include <memory>
#include "../components/RaspiOverlayRenderer.h"
#include "MotionFrame.h"
#include <opencv2/core.hpp>

namespace rpi_motioncam {
    class FramePreview {
        public: 
            FramePreview(RPIMOTIONCAM_OPTION_S options);
            void draw(const MotionFrame &frame);
        protected:
            RPIMOTIONCAM_OPTION_S options_;
            shared_ptr< RaspiOverlayRenderer > renderer_;
            float width_scale;
            float height_scale;
    };
}


#endif

