/*
Copyright (c) 2013, Broadcom Europe Ltd
Copyright (c) 2013, James Hughes
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "RaspiRenderer.h"

namespace raspivid {
    const char* RaspiRenderer::component_name() {
        return MMAL_COMPONENT_DEFAULT_VIDEO_RENDERER;
    }

    RaspiRenderer* RaspiRenderer::create(int alpha, int layer) {
        RaspiRenderer* result = new RaspiRenderer();
        result->alpha_ = alpha;
        result->layer_ = layer;
        if (result->init() != MMAL_SUCCESS) {
            result->destroy();
            return NULL;
        }
        return result;
    }

    RaspiRenderer* RaspiRenderer::create() {
        return create(255, PREVIEW_LAYER);
    }

    /**
     * Create the preview component, set up its ports
     *
     * @param state Pointer to state control struct
     *
     * @return MMAL_SUCCESS if all OK, something else otherwise
     *
     */
    MMAL_STATUS_T RaspiRenderer::init() {
        MMAL_STATUS_T status;

        if ((status = RaspiComponent::init()) != MMAL_SUCCESS) {
            return status;
        }

        assert_ports(1, 0);

        input = component->input[0];

        MMAL_DISPLAYREGION_T param;
        param.hdr.id = MMAL_PARAMETER_DISPLAYREGION;
        param.hdr.size = sizeof(MMAL_DISPLAYREGION_T);

        param.set = MMAL_DISPLAY_SET_LAYER;
        param.layer = layer_;

        param.set |= MMAL_DISPLAY_SET_ALPHA;
        param.alpha = alpha_;

        param.set |= MMAL_DISPLAY_SET_FULLSCREEN;
        param.fullscreen = 1;

        status = mmal_port_parameter_set(input, &param.hdr);

        if (status != MMAL_SUCCESS && status != MMAL_ENOSYS) {
             vcos_log_error("RaspiRenderer::init(): unable to set renderer port parameters (%u)", status);
             return status;
        }

        /* Enable component */
        if ((status = mmal_component_enable(component)) != MMAL_SUCCESS) {
            vcos_log_error("RaspiRenderer::init(): unable to enable renderer component (%u)", status);
            return status;
        }

    }


    /**
     * Destroy the preview component
     *
     * @param state Pointer to state control struct
     *
     */
    void RaspiRenderer::destroy() {
        RaspiComponent::destroy();
        check_disable_port(input);
    }

}
