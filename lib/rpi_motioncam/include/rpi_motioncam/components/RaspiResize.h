#ifndef __RASPIRESIZE_H__
#define __RASPIRESIZE_H__

#include <memory>
#include "rpi_motioncam/components/RaspiComponent.h"
#include "rpi_motioncam/RaspiPort.h"


namespace rpi_motioncam {
      
    class RaspiResize : public RaspiComponent {
        public:
            static shared_ptr< RaspiResize > create();
            shared_ptr< RaspiPort > input;
            shared_ptr< RaspiPort > output;
            MMAL_STATUS_T set_output(int width, int height);
        protected:
            const char* component_name();
            MMAL_STATUS_T init();
    };
}

#endif /* __RASPIRESIZE_H__ */
