#ifndef __RASPINULLSINK_H__
#define __RASPINULLSINK_H__

#include <memory>
#include "rpi_motioncam/components/RaspiComponent.h"
#include "rpi_motioncam/RaspiPort.h"

namespace rpi_motioncam {

    class RaspiNullsink : public RaspiComponent {
        public:
            static shared_ptr< RaspiNullsink > create();
            shared_ptr< RaspiPort > input;
        protected:
            const char* component_name();
            MMAL_STATUS_T init();
    };
}

#endif /* RASPIRENDERER_H_ */
