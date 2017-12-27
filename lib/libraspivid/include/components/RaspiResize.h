#ifndef __RASPIRESIZE_H__
#define __RASPIRESIZE_H__

#include <memory>
#include "components/RaspiComponent.h"
#include "RaspiPort.h"


namespace raspivid {
      
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
