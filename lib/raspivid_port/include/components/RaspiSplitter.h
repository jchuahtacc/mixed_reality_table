#ifndef __RASPISPLITTER_H__
#define __RASPISPLITTER_H__

#include "components/RaspiComponent.h"
#include "RaspiPort.h"


#define VIDEO_FRAME_RATE_NUM 30
#define VIDEO_FRAME_RATE_DEN 1

namespace raspivid {
      
    class RaspiSplitter : public RaspiComponent {
        public:
            static RaspiSplitter* create();
            RaspiPort *input;
            RaspiPort *output_0;
            RaspiPort *output_1;
            MMAL_STATUS_T duplicate_input(); 
            void destroy();
        protected:
            const char* component_name();
            MMAL_STATUS_T init();
    };
}

#endif /* __RASPISPLITTER_H__ */
