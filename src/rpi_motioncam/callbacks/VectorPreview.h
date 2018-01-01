#ifndef __VECTORPREVIEW_H__
#define __VECTORPREVIEW_H__

#include "../typedefs.h"
#include <memory>
#include "../components/RaspiOverlayRenderer.h"
#include <opencv2/core.hpp>

namespace rpi_motioncam {
    class VectorPreview {
        public: 
            VectorPreview(RPIMOTIONCAM_OPTION_S options);
            int render_buffer_position(int row, int col, int row_offset);
            void draw(char* lastBuffer);
        protected:
            RPIMOTIONCAM_OPTION_S options_;
            shared_ptr< RaspiOverlayRenderer > renderer_;
            int scanline_bytes;
            int row_bytes;
            int col_bytes;
            int cols_;
            int rows_;
    };
}


#endif 

