#include "RaspiPort.h"

namespace raspivid {

    RaspiPort::~RaspiPort() {
        destroy();
    }

    void RaspiPort::destroy() {
        if (connection) {
            mmal_connection_destroy(connection);
            connection = NULL;
        } else {
            if (port && port->is_enabled) {
                mmal_port_disable(port);
            }
            if (pool) {
                mmal_port_pool_destroy(port, pool);
                pool = NULL;
            }
        }
    }

    RaspiPort::RaspiPort(MMAL_PORT_T *mmal_port) {
        port = mmal_port;
    }

    RASPIPORT_FORMAT_S RaspiPort::createDefaultPortFormat() {
        RASPIPORT_FORMAT_S result;
        result.encoding = MMAL_ENCODING_OPAQUE;
        result.encoding_variant = MMAL_ENCODING_I420;
        result.width = 1920;
        result.height = 1080;
        result.crop_x = 0;
        result.crop_y = 0;
        result.crop_width = 0;
        result.crop_height = 0;
        result.frame_rate_num = 0;
        result.frame_rate_den = 1;
    }

    MMAL_STATUS_T RaspiPort::set_format(RASPIPORT_FORMAT_S options) {
        vcos_assert(port);
        MMAL_ES_FORMAT_T *format = port->format;
        format->encoding = options.encoding;
        format->encoding_variant = options.encoding_variant;
        format->es->video.width = VCOS_ALIGN_UP(options.width, 32);
        format->es->video.height = VCOS_ALIGN_UP(options.height, 16);
        format->es->video.crop.x = options.crop_x;
        format->es->video.crop.y = options.crop_y;
        format->es->video.crop.width = options.crop_width ? options.crop_width : options.width;
        format->es->video.crop.height = options.crop_height ? options.crop_height : options.height;
        format->es->video.frame_rate.num = options.frame_rate_num;
        format->es->video.frame_rate.den = options.frame_rate_den;

        MMAL_STATUS_T status;
        if ((status = mmal_port_format_commit(port)) != MMAL_SUCCESS) {
            vcos_log_error("RaspiPort::format(): unable to commit port format");
            return status;
        }
        return MMAL_SUCCESS;
    }

    RASPIPORT_FORMAT_S RaspiPort::get_format() {
        vcos_assert(port);
        MMAL_ES_FORMAT_T *format = port->format;
        RASPIPORT_FORMAT_S result;
        result.encoding = format->encoding;
        result.encoding_variant = format->encoding_variant;
        result.width = format->es->video.width;
        result.height = format->es->video.height;
        result.crop_x = format->es->video.crop.x;
        result.crop_y = format->es->video.crop.y;
        result.crop_width = format->es->video.crop.width;
        result.crop_height = format->es->video.crop.height;
        result.frame_rate_num = format->es->video.frame_rate.num;
        result.frame_rate_den = format->es->video.frame_rate.den;
        return result;
    }


    MMAL_STATUS_T RaspiPort::connect(MMAL_PORT_T *output_port, MMAL_CONNECTION_T **connection) {
        vcos_assert(output_port);
        vcos_assert(port);
        MMAL_STATUS_T status;
        mmal_format_copy(port->format, output_port->format);
        if ((status = mmal_connection_create(connection, output_port, port, MMAL_CONNECTION_FLAG_TUNNELLING | MMAL_CONNECTION_FLAG_ALLOCATION_ON_INPUT)) != MMAL_SUCCESS) {
            vcos_log_error("RaspiPort::connect(): unable to connect port");
            return status;
        }

        if ((status = mmal_connection_enable(*connection)) != MMAL_SUCCESS) {
            vcos_log_error("RaspiPort::connect(): unable to enable connection");
            mmal_connection_destroy(*connection);
            connection = NULL;
            return status;
        }

        return MMAL_SUCCESS;
    }

    MMAL_STATUS_T RaspiPort::connect(RaspiPort *output_port) {
        return connect(output_port->port, &connection);
    }

    void RaspiPort::callback_wrapper(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
        RASPIPORT_USERDATA_S *userdata = (RASPIPORT_USERDATA_S *)port->userdata;
        vcos_assert(userdata);
        userdata->cb_instance->callback(port, buffer);
        MMAL_POOL_T *pool = userdata->pool;
        mmal_buffer_header_release(buffer);
        if (pool && port->is_enabled) {
            MMAL_BUFFER_HEADER_T *new_buffer = mmal_queue_get(pool->queue);
            if (new_buffer) {
                if (mmal_port_send_buffer(port, new_buffer) != MMAL_SUCCESS) {
                    vcos_log_error("RaspiPort::callback_wrapper(): unable to return a buffer");
                }
            }
        }
    }

    MMAL_STATUS_T RaspiPort::add_callback(RaspiCallback *cb_instance) {
        port->userdata = (struct MMAL_PORT_USERDATA_T *)&userdata;
        
        userdata.cb_instance = cb_instance;

        MMAL_STATUS_T status;

        if ((status = mmal_port_enable(port, callback_wrapper)) != MMAL_SUCCESS) {
            vcos_log_error("RaspiPort::enable(): unable to setup callback on port");
            return status;
        }

        pool = mmal_port_pool_create(port, port->buffer_num, port->buffer_size);
        if (!pool) {
            vcos_log_error("RaspiPort::add_callback(): unable to create buffer header pool");
        }
        userdata.pool = pool;
        int queue_length = mmal_queue_length(pool->queue);
        for (int i = 0; i < queue_length; i++) {
            MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(pool->queue);
            if (!buffer) {
                vcos_log_error("RaspiPort:add_callback(): unable to get buffer from pool");
            }
            if (mmal_port_send_buffer(port, buffer) != MMAL_SUCCESS) {
                vcos_log_error("RaspiPort::add_callback(): unable to send buffer to output port");
            }
        }


        return MMAL_SUCCESS;
    }
}
