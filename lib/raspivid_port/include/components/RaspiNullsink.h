#ifndef __RASPINULLSINK_H__
#define __RASPINULLSINK_H__

#include "components/RaspiComponent.h"
#include "RaspiPort.h"

namespace raspivid {

    class RaspiNullsink : public RaspiComponent {
        public:
            static RaspiNullsink* create();
            RaspiPort *input;
            void destroy();
        protected:
            const char* component_name();
            MMAL_STATUS_T init();
    };
}

#endif /* RASPIRENDERER_H_ */
