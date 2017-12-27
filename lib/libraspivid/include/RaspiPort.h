#ifndef __RASPIPORT_H__
#define __RASPIPORT_H__

#include <memory>
#include <string>
#include "RaspiCallback.h"

using namespace std;

namespace raspivid {
    typedef struct {
        shared_ptr< RaspiCallback > cb_instance;
        MMAL_POOL_T* pool;
    } RASPIPORT_USERDATA_S;

    typedef struct {
        uint32_t encoding;
        uint32_t encoding_variant;
        uint32_t width;
        uint32_t height;
        MMAL_RECT_T crop;
        uint32_t frame_rate_num;
        uint32_t frame_rate_den;
    } RASPIPORT_FORMAT_S;

    class RaspiPort {
        public:
            static RASPIPORT_FORMAT_S createDefaultPortFormat();
            static shared_ptr< RaspiPort > create(MMAL_PORT_T *port, string port_name_ = "port_name");
            MMAL_STATUS_T set_format(RASPIPORT_FORMAT_S new_format);
            RASPIPORT_FORMAT_S get_format();
            MMAL_STATUS_T add_callback(shared_ptr< RaspiCallback > callback);
            MMAL_STATUS_T connect(shared_ptr< RaspiPort > output);
            MMAL_STATUS_T connect(MMAL_PORT_T *output, MMAL_CONNECTION_T **connection);
            MMAL_STATUS_T create_buffer_pool();
            MMAL_BUFFER_HEADER_T* get_buffer();
            MMAL_STATUS_T send_buffer(MMAL_BUFFER_HEADER_T *buffer);
            MMAL_STATUS_T set_zero_copy();
            void destroy();
            ~RaspiPort();
        protected:
            RaspiPort(MMAL_PORT_T *port, string port_name_);
            string port_name;
        private:
            static void callback_wrapper(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
            RASPIPORT_USERDATA_S userdata;
            MMAL_POOL_T *pool;
            MMAL_PORT_T *port;
            MMAL_CONNECTION_T *connection;
    };

}


#endif /* __RASPIPORT_H__ */
