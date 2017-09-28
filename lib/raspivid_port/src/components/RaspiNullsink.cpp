#include "components/RaspiNullsink.h"

namespace raspivid {
    const char* RaspiNullsink::component_name() {
        return "vc.null_sink";
    }

    RaspiNullsink* RaspiNullsink::create() {
        RaspiNullsink* result = new RaspiNullsink();
        if (result->init() != MMAL_SUCCESS) {
            result->destroy();
            return NULL;
        }
        return result;
    }

    MMAL_STATUS_T RaspiNullsink::init() {
        MMAL_STATUS_T status;

        if ((status = RaspiComponent::init()) != MMAL_SUCCESS) {
            return status;
        }

        assert_ports(1, 0);
        if ((status = mmal_component_enable(component)) != MMAL_SUCCESS) {
            vcos_log_error("RaspiNullsink::init(): unable to enable nullsink component (%u)", status);
            return status;
        }

        input = new RaspiPort(component->input[0]);

        return MMAL_SUCCESS;
    }

    void RaspiNullsink::destroy() {
        if (input) {
            input->destroy();
            delete input;
        }
        RaspiComponent::destroy();
    }

}
