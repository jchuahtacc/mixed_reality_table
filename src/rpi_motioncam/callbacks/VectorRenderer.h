#ifndef __VECTORRENDERER_H__
#define __VECTORRENDERER_H__

#include "../typedefs.h"
#include <memory>
#include "MotionVectorCallback.h"
#include "../components/RaspiOverlayRenderer.h"
#include <opencv2/core.hpp>

namespace rpi_motioncam {
    class VectorRenderer : public RaspiOverlayRenderer {
        public: 
            static shared_ptr< VectorRenderer > create(RPI_MOTIONCAM_OPTION_S, char* lastBuffer_);
            int render_buffer_position(int row, int col, int row_offset);
            void draw();
        protected:
            char* lastBuffer;
            VectorRenderer(RASPIOVERLAYRENDERER_OPTION_S options, RPIMOTIONCAM_OPTION_S options, char* lastBuffer_);
            int scanline_bytes;
            int row_bytes;
            int col_bytes;
            RPIMOTIONCAM_OPTION_S options_;
    };
}


#endif /* __MOTIONVECTORPREVIEWCALLBACK_H__ */

