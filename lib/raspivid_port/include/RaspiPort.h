#ifndef __RASPIPORT_H__
#define __RASPIPORT_H__

#include "components/RaspiComponent.h"
#include "RaspiCallback.h"

namespace raspivid {
    typedef struct {
        RaspiCallback* cb_instance;
        MMAL_POOL_T* pool;
    } RASPIPORT_USERDATA_S;

    class RaspiPort {
        public:
            RaspiPort(MMAL_PORT_T *port);
            ~RaspiPort();
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
