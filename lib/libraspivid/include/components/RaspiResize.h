#ifndef __RASPIRESIZE_H__
#define __RASPIRESIZE_H__

#include <memory>
#include "components/RaspiComponent.h"
#include "RaspiPort.h"


namespace raspivid {
      
    class RaspiResize : public RaspiComponent {
        public:
            static shared_ptr< RaspiResize > create(int width, int height);
            shared_ptr< RaspiPort > input;
            shared_ptr< RaspiPort > output;
            MMAL_STATUS_T connect( shared_ptr< RaspiComponent > source_component );
            MMAL_STATUS_T connect( shared_ptr< RaspiPort > source_port );
        protected:
            const char* component_name();
            MMAL_STATUS_T init();
            int width_;
            int height_;
    };
}

#endif /* __RASPIRESIZE_H__ */
