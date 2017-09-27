#ifndef __RASPINULLSINK_H__
#define __RASPINULLSINK_H__

#include "RaspiComponent.h"

namespace raspivid {

    class RaspiNullsink : RaspiComponent {
        public:
            static RaspiNullsink* create();
            MMAL_PORT_T *input;
            void destroy();
        protected:
            const char* component_name();
            MMAL_STATUS_T init();
    };
}

#endif /* RASPIRENDERER_H_ */
