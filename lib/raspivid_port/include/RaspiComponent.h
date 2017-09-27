#ifndef __RASPICOMPONENT_H__
#define __RASPICOMPONENT_H__ 

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <memory.h>

#include "interface/vcos/vcos.h"

#include "interface/mmal/mmal.h"
#include "interface/mmal/mmal_logging.h"
#include "interface/mmal/mmal_buffer.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"
#include "interface/mmal/mmal_parameters_camera.h"

using namespace std;

namespace raspivid {
    class RaspiComponent {
        public:
            ~RaspiComponent();
            MMAL_PORT_T* input;
            void destroy();
            MMAL_CONNECTION_T* connect(MMAL_PORT_T* output);
        protected:
            RaspiComponent();
            MMAL_STATUS_T init();
            MMAL_COMPONENT_T *component;
            virtual const char* component_name() =0;
            void assert_ports(int inputs, int outputs);
            void check_disable_port(MMAL_PORT_T* port);
    };
}

#endif /* __RASPICOMPONENT_H__  */
