#include "components/RaspiSplitter.h"

namespace raspivid {
    const char* RaspiSplitter::component_name() {
        return MMAL_COMPONENT_DEFAULT_VIDEO_SPLITTER;
    }

    RaspiSplitter* RaspiSplitter::create() {
        RaspiSplitter* result = new RaspiSplitter();
        if (result->init() != MMAL_SUCCESS) {
            result->destroy();
            return NULL;
        }
        return result;
    }

    MMAL_STATUS_T RaspiSplitter::init() {
        MMAL_STATUS_T status;

        if ((status = RaspiComponent::init()) != MMAL_SUCCESS) {
            return status;
        }

        assert_ports(1, 2);

        MMAL_PORT_T *mmal_input = component->input[0];
        MMAL_PORT_T *mmal_output_0 = component->output[0];
        MMAL_PORT_T *mmal_output_1 = component->output[1];

        input = new RaspiPort(mmal_input);
        output_0 = new RaspiPort(mmal_output_0);
        output_1 = new RaspiPort(mmal_output_1);

        vcos_log_error("RaspiSplitter::init(): success!");

        return MMAL_SUCCESS;
    }

    MMAL_STATUS_T RaspiSplitter::duplicate_input() {
        vcos_assert(input);
        vcos_assert(output_0);
        vcos_assert(output_1);
        MMAL_STATUS_T status;
        if ((status = output_0->set_format(input->get_format())) != MMAL_SUCCESS) {
            vcos_log_error("RaspiSplitter::connect(): Unable to set output_0 format to input format");
            return status;
        }
        if ((status = output_1->set_format(input->get_format())) != MMAL_SUCCESS) {
            vcos_log_error("RaspiSplitter::connect(): Unable to set output_1 format to input format");
            return status;
        }

        return MMAL_SUCCESS;
    }

    void RaspiSplitter::destroy() {
        if (input) {
            input->destroy();
            delete input;
        }
        if (output_0) {
            output_0->destroy();
            delete output_0;
        }
        if (output_1) {
            output_1->destroy();
            delete output_1;
        }
        RaspiComponent::destroy();
    }

}
