#ifndef __RASPIRENDERER_H__
#define __RASPIRENDERER_H__

#include <memory>
#include "rpi_motioncam/components/RaspiComponent.h"
#include "rpi_motioncam/RaspiPort.h"

/// Layer that preview window should be displayed on
#define PREVIEW_LAYER      2

// Frames rates of 0 implies variable, but denominator needs to be 1 to prevent div by 0
#define PREVIEW_FRAME_RATE_NUM 0
#define PREVIEW_FRAME_RATE_DEN 1

#define FULL_RES_PREVIEW_FRAME_RATE_NUM 0
#define FULL_RES_PREVIEW_FRAME_RATE_DEN 1

#define FULL_FOV_PREVIEW_16x9_X 1280
#define FULL_FOV_PREVIEW_16x9_Y 720

#define FULL_FOV_PREVIEW_4x3_X 1296
#define FULL_FOV_PREVIEW_4x3_Y 972

#define FULL_FOV_PREVIEW_FRAME_RATE_NUM 0
#define FULL_FOV_PREVIEW_FRAME_RATE_DEN 1

namespace rpi_motioncam {

    class RaspiRenderer : public RaspiComponent {
        public:
            static shared_ptr< RaspiRenderer > create(int alpha, int layer);
            static shared_ptr< RaspiRenderer > create();
            shared_ptr< RaspiPort > input;
        protected:
            const char* component_name();
            int alpha_ = 255;
            int layer_ = PREVIEW_LAYER;
            MMAL_STATUS_T init();
            MMAL_RECT_T rendererWindow;
    };
}

#endif /* RASPIRENDERER_H_ */
