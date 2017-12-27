#ifndef __RASPICOMPONENT_H__
#define __RASPICOMPONENT_H__ 

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <memory.h>
#include "RaspiPort.h"

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
            void destroy();
            MMAL_STATUS_T connect(shared_ptr< RaspiComponent > source_component);
            MMAL_STATUS_T connect(shared_ptr< RaspiPort > source_port);
            shared_ptr< RaspiPort > default_input = nullptr;
            shared_ptr< RaspiPort > default_output = nullptr;
        protected:
            RaspiComponent();
            MMAL_STATUS_T init();
            MMAL_COMPONENT_T *component;
            virtual const char* component_name() =0;
            void assert_ports(int inputs, int outputs);
    };
}

#endif /* __RASPICOMPONENT_H__  */
