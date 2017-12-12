#ifndef __RASPIOVERLAYRENDERER_H__
#define __RASPIOVERLAYRENDERER_H__

#include "components/RaspiComponent.h"
#include "RaspiPort.h"
#include "RaspiRenderer.h"
#include "RaspiCallback.h"

namespace raspivid {

    typedef struct {
        uint32_t encoding;
        uint32_t width;
        uint32_t height;
        uint32_t layer;
        uint32_t alpha;
        MMAL_BOOL_T fullscreen;
        MMAL_RECT_T crop;
        MMAL_RECT_T dest;
    } RASPIOVERLAYRENDERER_FORMAT_S;

    class RaspiOverlayRenderer : public RaspiRenderer {
        public:
            void destroy();
            MMAL_BUFFER_HEADER_T* get_buffer();
            void send_buffer(MMAL_BUFFER_HEADER_T *buffer);
            static RASPIOVERLAYRENDERER_FORMAT_S createDefaultOverlayFormat();
            static RaspiOverlayRenderer* create();
            static RaspiOverlayRenderer* create(RASPIOVERLAYRENDERER_FORMAT_S format);
        protected:
            MMAL_STATUS_T init();
            RASPIOVERLAYRENDERER_FORMAT_S format_;
            static void callback_(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
    };
}

#endif /* RASPIOVERLAYRENDERER_H_ */
