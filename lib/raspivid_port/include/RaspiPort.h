#ifndef __RASPIPORT_H__
#define __RASPIPORT_H__

#include "components/RaspiComponent.h"
#include "RaspiCallback.h"

namespace raspivid {
    typedef struct {
        RaspiCallback* cb_instance;
        MMAL_POOL_T* pool;
    } RASPIPORT_USERDATA_S;

    typedef struct {
        uint32_t encoding;
        uint32_t encoding_variant;
        uint32_t width;
        uint32_t height;
        uint32_t crop_x;
        uint32_t crop_y;
        uint32_t crop_width;
        uint32_t crop_height;
        uint32_t frame_rate_num;
        uint32_t frame_rate_den;
    } RASPIPORT_FORMAT_S;

    class RaspiPort {
        public:
            RaspiPort(MMAL_PORT_T *port);
            ~RaspiPort();
            static RASPIPORT_FORMAT_S createDefaultPortFormat();
            MMAL_STATUS_T set_format(RASPIPORT_FORMAT_S new_format);
            RASPIPORT_FORMAT_S get_format();
            MMAL_STATUS_T add_callback(RaspiCallback *callback);
            MMAL_STATUS_T connect(RaspiPort *output);
            MMAL_STATUS_T connect(MMAL_PORT_T *output, MMAL_CONNECTION_T **connection);
            void destroy();
        private:
            static void callback_wrapper(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
            RASPIPORT_USERDATA_S userdata;
            MMAL_POOL_T *pool;
            MMAL_PORT_T *port;
            MMAL_CONNECTION_T *connection;
    };

}


#endif /* __RASPIPORT_H__ */
