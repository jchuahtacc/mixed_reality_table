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


    MMAL_STATUS_T RaspiPort::connect(MMAL_PORT_T *output_port, MMAL_CONNECTION_T **connection) {
        vcos_assert(output_port);
        vcos_assert(port);
        MMAL_STATUS_T status = mmal_connection_create(connection, output_port, port, MMAL_CONNECTION_FLAG_TUNNELLING | MMAL_CONNECTION_FLAG_ALLOCATION_ON_INPUT);
        if (status != MMAL_SUCCESS) {
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
        /*
        vcos_assert(port);
        MMAL_STATUS_T status = mmal_connection_create(&connection, output_port->port, port, MMAL_CONNECTION_FLAG_TUNNELLING | MMAL_CONNECTION_FLAG_ALLOCATION_ON_INPUT);
        if (status != MMAL_SUCCESS) {
            vcos_log_error("RaspiPort::connect(): unable to connect port");
            return status;
        }

        if ((status = mmal_connection_enable(connection)) != MMAL_SUCCESS) {
            vcos_log_error("RaspiPort::connect(): unable to enable connection");
            mmal_connection_destroy(connection);
            connection = NULL;
            return status;
        }

        return MMAL_SUCCESS;
        */
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
