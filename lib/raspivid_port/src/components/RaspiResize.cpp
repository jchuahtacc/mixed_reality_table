#include "components/RaspiResize.h"

namespace raspivid {
    const char* RaspiResize::component_name() {
        return "vc.ril.resize";
    }

    RaspiResize* RaspiResize::create() {
        RaspiResize* result = new RaspiResize();
        if (result->init() != MMAL_SUCCESS) {
            result->destroy();
            return NULL;
        }
        return result;
    }

    MMAL_STATUS_T RaspiResize::init() {
        MMAL_STATUS_T status;

        if ((status = RaspiComponent::init()) != MMAL_SUCCESS) {
            return status;
        }

        assert_ports(1, 1);

        MMAL_PORT_T *mmal_input = component->input[0];
        MMAL_PORT_T *mmal_output = component->output[0];

        input = new RaspiPort(mmal_input);
        output = new RaspiPort(mmal_output);

        vcos_log_error("RaspiResize::init(): success!");

        return MMAL_SUCCESS;
    }

    MMAL_STATUS_T RaspiResize::set_output(int width, int height) {
        RASPIPORT_FORMAT_S format = input->get_format();
        format.encoding = MMAL_ENCODING_I420;
        format.encoding_variant = MMAL_ENCODING_I420;
        format.width = VCOS_ALIGN_UP(width, 32);
        format.height = VCOS_ALIGN_UP(height, 16);
        format.crop_width = width;
        format.crop_height = height;
        MMAL_STATUS_T status;
        if ((status = output->set_format(format)) != MMAL_SUCCESS) {
            vcos_log_error("RaspiResize::set_output(): unable to set output size format");
            return status;
        }
        return MMAL_SUCCESS;
    }

    void RaspiResize::destroy() {
        if (input) {
            input->destroy();
            delete input;
        }
        if (output) {
            output->destroy();
            delete output;
        }
        RaspiComponent::destroy();
    }

}
