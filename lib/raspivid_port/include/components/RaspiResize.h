#ifndef __RASPIRESIZE_H__
#define __RASPIRESIZE_H__

#include "components/RaspiComponent.h"
#include "RaspiPort.h"


namespace raspivid {
      
    class RaspiResize : public RaspiComponent {
        public:
            static RaspiResize* create();
            RaspiPort *input;
            RaspiPort *output;
            void destroy();
            MMAL_STATUS_T set_output(int width, int height);
        protected:
            const char* component_name();
            MMAL_STATUS_T init();
    };
}

#endif /* __RASPIRESIZE_H__ */
