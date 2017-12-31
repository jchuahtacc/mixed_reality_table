#ifndef __RASPISPLITTER_H__
#define __RASPISPLITTER_H__

#include <memory>
#include "rpi_motioncam/components/RaspiComponent.h"
#include "rpi_motioncam/RaspiPort.h"


#define VIDEO_FRAME_RATE_NUM 30
#define VIDEO_FRAME_RATE_DEN 1

namespace rpi_motioncam {
      
    class RaspiSplitter : public RaspiComponent {
        public:
            static shared_ptr< RaspiSplitter > create();
            shared_ptr< RaspiPort > input;
            shared_ptr< RaspiPort > output_0;
            shared_ptr< RaspiPort > output_1;
            MMAL_STATUS_T duplicate_input(); 
        protected:
            const char* component_name();
            MMAL_STATUS_T init();
    };
}

#endif /* __RASPISPLITTER_H__ */
