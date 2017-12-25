#ifndef __RASPINULLSINK_H__
#define __RASPINULLSINK_H__

#include <memory>
#include "components/RaspiComponent.h"
#include "RaspiPort.h"

namespace raspivid {

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
