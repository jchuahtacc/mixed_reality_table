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
            int buffer_position(int row, int col, int row_offset);
            void post_process();
        protected:
            shared_ptr< RaspiOverlayRenderer > renderer_;
            char red[3] = { 255, 0, 0 };
            char green[3] = { 0, 255, 0 };
            char blue[3] = { 0, 0, 255 };
            
            vector< char* > colors;
    };
}


#endif /* __MOTIONVECTORPREVIEWCALLBACK_H__ */

