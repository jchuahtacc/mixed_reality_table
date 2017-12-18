#ifndef __MOTIONVECTORCALLBACK_H__
#define __MOTIONVECTORCALLBACK_H__

#include <memory>
#include "MotionVectorCallback.h"
#include "components/RaspiOverlayRenderer.h"

namespace raspivid {
    class MotionVectorPreviewCallback : public MotionVectorCallback {
        public: 
            void callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
            MotionVectorPreviewCallback(uint16_t width, uint16_t height, shared_ptr< RaspiOverlayRenderer > renderer);
        protected:
            shared_ptr< RaspiOverlayRenderer > renderer_;
    };
}


#endif /* __MOTIONVECTORCALLBACK_H__ */

