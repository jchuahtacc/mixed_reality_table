#ifndef __RASPICALLBACK_H__
#define __RASPICALLBACK_H__

#include "bcm_host.h"
#include "interface/vcos/vcos.h"

#include "interface/mmal/mmal.h"
#include "interface/mmal/mmal_logging.h"
#include "interface/mmal/mmal_buffer.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"
#include "interface/mmal/mmal_parameters_camera.h"


namespace raspivid {
    class RaspiCallback {
        public:
            virtual void callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) =0;
    };
}


#endif /* __RASPICALLBACK_H__ */
