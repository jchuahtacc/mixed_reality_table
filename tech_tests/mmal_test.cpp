#include <bcm_host.h>
#include <interface/mmal/mmal.h>
#include <interface/mmal/mmal_logging.h>
#include <interface/mmal/mmal_buffer.h>
#include <interface/mmal/util/mmal_util.h>
#include <interface/mmal/util/mmal_util_params.h>
#include <interface/mmal/util/mmal_default_components.h>
#include <interface/mmal/util/mmal_util_params.h>
#include <interface/mmal/util/mmal_connection.h>
#include <iostream>
#include <interface/vcos/vcos.h>



#define MMAL_CAMERA_PREVIEW_PORT 0
#define MMAL_CAMERA_VIDEO_PORT 1
#define MMAL_CAMERA_STILL_PORT 2
#define MMAL_ENCODING_OPAQUE MMAL_FOURCC('O','P','Q','V')
#define MMAL_ENCODING_I420 MMAL_FOURCC('I','4','2','0')
#define MMAL_ENCODING_H264 MMAL_FOURCC('H','2','6','4')


using namespace std;

const int width = 640;
const int height = 640;
const int MAX_BITRATE_LEVEL4 = 25000000;
const int framerate = 30;

MMAL_COMPONENT_T *camera = 0;
MMAL_COMPONENT_T *splitter = 0;
MMAL_COMPONENT_T *encoder = 0;
MMAL_POOL_T *splitter_pool = 0;
MMAL_POOL_T *encoder_pool = 0;

// Callback for camera settings control
// *port is pointer to port making callback
// *buffer is pointer to data passed in the callback
static void camera_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
    // Check the buffer's cmd field. This should indicate the type of event
    if (buffer->cmd == MMAL_EVENT_PARAMETER_CHANGED) {
        // Since this event is a "PARAMETER CHANGED" event, the data can be casted to an MMAL_EVENT_PARAMETER_CHANET_T struct with stuff in it
        MMAL_EVENT_PARAMETER_CHANGED_T *param = (MMAL_EVENT_PARAMETER_CHANGED_T *)buffer->data;
        cout << "camera_control_callback success " << endl;
    } else if (buffer->cmd == MMAL_EVENT_ERROR) {
        cout << "camera_control_callback error: no data received from sensor. Is it plugged in?" << endl;
    } else {
        cout << "camera_control_callback error: unexpecetd callback event " << buffer->cmd << endl;
    }
    // Release the buffer when done with the callback!!!!
    mmal_buffer_header_release(buffer);
}

void cleanup() {
    cout << "Starting cleanup" << endl;
    mmal_component_destroy(camera);
    mmal_port_pool_destroy(splitter->output[0], splitter_pool);
    mmal_component_destroy(splitter);
    mmal_port_pool_destroy(encoder->output[0], encoder_pool);
    mmal_component_destroy(encoder);
    cout << "Cleanup finished! Exiting... " << endl;
}

void ABORT_IF_ERROR(MMAL_STATUS_T status) {
    switch(status) {
        case MMAL_SUCCESS : return;
        case MMAL_ENOMEM : {
                               cout << "MMAL_STATUS_T Error: MMAL_ENOMEM (Out of Memory)" << endl; cleanup(); exit(-1);
                           }
        case MMAL_ENOSPC : {
                               cout << "MMAL_STATUS_T Error: MMAL_ENOSPC (Out of resources)" << endl;  cleanup();exit(-1);
                           }
        case MMAL_EINVAL : {
                               cout << "MMAL_STATUS_T Error: MMAL_EINVAL (Argument is invalid)" << endl;  cleanup();exit(-1);
                           }
        case MMAL_ENOSYS : {
                               cout << "MMAL_STATUS_T Error: MMAL_ENOSYS (Function not implemented)" << endl;  cleanup();exit(-1);
                           }
        case MMAL_ENOENT : {
                               cout << "MMAL_STATUS_T Error: MMAL_ENOENT (No such file or directory)" << endl;  cleanup();exit(-1);
                           }
        case MMAL_ENXIO : {
                               cout << "MMAL_STATUS_T Error: MMAL_ENXIO (No such device or address)" << endl;  cleanup();exit(-1);
                           }
        case MMAL_EIO : {
                               cout << "MMAL_STATUS_T Error: MMAL_EIO (I/O Error)" << endl;  cleanup();exit(-1);
                           }
        case MMAL_ESPIPE : {
                               cout << "MMAL_STATUS_T Error: MMAL_ESPIPE (Illegal seek)" << endl;  cleanup();exit(-1);
                           }
        case MMAL_ECORRUPT : {
                               cout << "MMAL_STATUS_T Error: MMAL_ECORRUPT (Data is corrupt)" << endl;  cleanup();exit(-1);
                           }
        case MMAL_ENOTREADY : {
                               cout << "MMAL_STATUS_T Error: MMAL_ENOTREADY (Component is not ready)" << endl;  cleanup();exit(-1);
                           }
        case MMAL_EISCONN : {
                               cout << "MMAL_STATUS_T Error: MMAL_EISCONN (Port is already connected)" << endl;  cleanup();exit(-1);
                           }
        case MMAL_EAGAIN : {
                               cout << "MMAL_STATUS_T Error: MMAL_EAGAIN (Resource temporarily unavailable. Try again later.)" << endl;  cleanup();exit(-1);
                           }
        case MMAL_EFAULT : {
                               cout << "MMAL_STATUS_T Error: MMAL_EFAULT (Bad address)" << endl;  cleanup();exit(-1);
                           }
        case MMAL_STATUS_MAX : {
                               cout << "MMAL_STATUS_T Error: MMAL_STATUS_MAX (Force to 32 bit)" << endl;  cleanup();exit(-1);
                           }
        default : {
                               cout << "MMAL_STATUS_T Error: (Unknown!)" << endl;  cleanup();exit(-1);
                           }
 
    }
}

void init_camera() {
    MMAL_STATUS_T status;
    cout << "Creating camera" << endl;
    // Make the camera component
    status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera);
    ABORT_IF_ERROR(status);

    // Change camera parameters
    // Create an MMAL_PARAMETER int32 struct with the value 0
    MMAL_PARAMETER_INT32_T camera_num = {{MMAL_PARAMETER_CAMERA_NUM, sizeof(camera_num)}, 0};
    cout << "Setting camera to camera 0" << endl;
    // set the port parameter
    status = mmal_port_parameter_set(camera->control, &camera_num.hdr);
    ABORT_IF_ERROR(status);

    if (!camera->output_num) {
        cout << "Camera doesn't have output ports!" << endl;
        ABORT_IF_ERROR(MMAL_ENOSYS);
    }

    cout << "Settting camera sensor mode to auto (0)" << endl;
    status = mmal_port_parameter_set_uint32(camera->control, MMAL_PARAMETER_CAMERA_CUSTOM_SENSOR_CONFIG, 0);
    ABORT_IF_ERROR(status);

    // Getting camera output ports
    MMAL_PORT_T* preview_port = camera->output[MMAL_CAMERA_PREVIEW_PORT];
    MMAL_PORT_T* video_port = camera->output[MMAL_CAMERA_VIDEO_PORT];
    MMAL_PORT_T* still_port = camera->output[MMAL_CAMERA_STILL_PORT];
   
    // Requests a callback on parameter change
    MMAL_PARAMETER_CHANGE_EVENT_REQUEST_T change_event_request = 
        {{MMAL_PARAMETER_CHANGE_EVENT_REQUEST, sizeof(MMAL_PARAMETER_CHANGE_EVENT_REQUEST_T)}, MMAL_PARAMETER_CAMERA_SETTINGS, 1};
    status = mmal_port_parameter_set(camera->control, &change_event_request.hdr);
    if (status != MMAL_SUCCESS) {
        cout << "No camera settings events" << endl;
    }

    // Starts the camera, with a callback for camera control events
    cout << "Starting camera" << endl;
    status = mmal_port_enable(camera->control, camera_control_callback);
    ABORT_IF_ERROR(status);

    // Actually configure the camera now that it's started
    cout << "Configuring camera resolution and framerate" << endl;
    {
        MMAL_PARAMETER_CAMERA_CONFIG_T cam_config =
        {
            { MMAL_PARAMETER_CAMERA_CONFIG, sizeof(cam_config) },
            .max_stills_w = width,
            .max_stills_h = height,
            .stills_yuv422 = 0,
            .one_shot_stills = 0,
            .max_preview_video_w = width,
            .max_preview_video_h = height,
            .num_preview_video_frames = 3,
            .stills_capture_circular_buffer_height = 0,
            .fast_preview_resume = 0,
            .use_stc_timestamp = MMAL_PARAM_TIMESTAMP_MODE_RAW_STC
        };
        mmal_port_parameter_set(camera->control, &cam_config.hdr);
    }

    // Setup port formats. Get whatever the preview port format is first
    // then change it to opaque format (direct gpu memory access)
    // with a subformat of YUV 420 (8-bit)
    MMAL_ES_FORMAT_T* format = preview_port->format;
    format->encoding = MMAL_ENCODING_OPAQUE;
    format->encoding_variant = MMAL_ENCODING_I420;

    // Set frame rate ranges (parens are for dirty variable scoping trick)
    
    cout << "Setting frame rate minimum 15, max... 1000 or 166? " << endl;
    MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)}, { 15, 1000 }, { 166, 1000 }};
    status = mmal_port_parameter_set(preview_port, &fps_range.hdr);
    if (status != MMAL_SUCCESS) {
        cout << "Error setting frame rate. Code: " << status << endl;
    }


    // no idea why it's setting this twice
    format->encoding = MMAL_ENCODING_OPAQUE;
    // Set format of elementary stream
    // VCOS_ALIGN_UP makes sure the values are multiples of 32 or 16, respectively
    format->es->video.width = VCOS_ALIGN_UP(width, 32);
    format->es->video.height = VCOS_ALIGN_UP(height, 16);
    format->es->video.crop.x = 0;
    format->es->video.crop.y = 0;
    format->es->video.crop.width = width;
    format->es->video.crop.height = height;
    format->es->video.frame_rate.num = framerate;  // frame rate numerator
    format->es->video.frame_rate.den = 1;   // frame rate denominator

    cout << "Setting preview port format" << endl;
    status = mmal_port_format_commit(preview_port);
    ABORT_IF_ERROR(status);

    // Reuse format struct. Setup the encoding on the video port now
    // Settings are same, but need default video_port settings first
    format = video_port->format;
    format->encoding_variant = MMAL_ENCODING_I420;
    // Reuse fps range
    mmal_port_parameter_set(video_port, &fps_range.hdr);
    format->es->video.width = VCOS_ALIGN_UP(width, 32);
    format->es->video.height = VCOS_ALIGN_UP(height, 16);
    format->es->video.crop.x = 0;
    format->es->video.crop.y = 0;
    format->es->video.crop.width = width;
    format->es->video.crop.height = height;
    format->es->video.frame_rate.num = framerate;     // can be separate frame rate from preview port
    format->es->video.frame_rate.den = 1;
    
    cout << "Setting video port format" << endl;
    status = mmal_port_format_commit(video_port);
    ABORT_IF_ERROR(status);

    format = still_port->format;
    format->encoding = MMAL_ENCODING_OPAQUE;
    format->encoding_variant = MMAL_ENCODING_I420;
    format->es->video.width = VCOS_ALIGN_UP(width, 32);
    format->es->video.height = VCOS_ALIGN_UP(height, 16);
    format->es->video.crop.x = 0;
    format->es->video.crop.y = 0;
    format->es->video.crop.width = width;
    format->es->video.crop.height = height; 
    format->es->video.frame_rate.num = 0;
    format->es->video.frame_rate.den = 1;
    cout << "Setting still port format" << endl;
    status = mmal_port_format_commit(still_port);
    ABORT_IF_ERROR(status);


    cout << "Camera setup complete!" << endl;


    // Make sure there are at least 3 buffers to prevent frame dropping
    video_port->buffer_num = 3;

}

void init_splitter() {
    MMAL_PORT_T *splitter_output = NULL;
    MMAL_ES_FORMAT_T *format;
    MMAL_STATUS_T status;
    MMAL_POOL_T *pool;

    cout << "Creating splitter component" << endl;
    status = mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_SPLITTER, &splitter);
    ABORT_IF_ERROR(status);

    if (splitter->output_num < 2) {
        cout << "Splitter doesn't have enough output ports" << endl;
        ABORT_IF_ERROR(MMAL_ENXIO);
    }

    // Make sure splitter input format matches camera
    mmal_format_copy(splitter->input[0]->format, camera->output[MMAL_CAMERA_PREVIEW_PORT]->format);

    splitter->input[0]->buffer_num = 3;
    cout << "Setting splitter input port format" << endl;
    status = mmal_port_format_commit(splitter->input[0]);
    ABORT_IF_ERROR(status);

    // Set grayscale foramt for all output ports. May regret this later. Droppign OPQV?
    cout << "Setting splitter output port formats" << endl;
    for (int i = 0; i < splitter->output_num; i++) {
        mmal_format_copy(splitter->output[i]->format, splitter->input[0]->format);
        format = splitter->output[i]->format;
        format->encoding = MMAL_ENCODING_I420;
        format->encoding_variant = MMAL_ENCODING_I420;
        status = mmal_port_format_commit(splitter->output[i]);
        ABORT_IF_ERROR(status);
    }

    cout << "Enabling splitter" << endl;
    status = mmal_component_enable(splitter);
    ABORT_IF_ERROR(status);

    // Create a pool of buffer headers for the output port to consume
    splitter_output = splitter->output[0];
    pool = mmal_port_pool_create(splitter_output, splitter_output->buffer_num, splitter_output->buffer_size);
    if (!pool) {
        cout << "Couldn't create buffers for the splitter output" << endl;
    }
    splitter_pool = pool;

}

void init_encoder() {
    MMAL_PORT_T *encoder_input = NULL, *encoder_output = NULL;
    MMAL_STATUS_T status;
    MMAL_POOL_T *pool;

    MMAL_VIDEO_LEVEL_T level = MMAL_VIDEO_LEVEL_H264_4;

    cout << "Creating encoder component" << endl;
    status = mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_ENCODER, &encoder);
    ABORT_IF_ERROR(status);

    encoder_input = encoder->input[0];
    encoder_output = encoder->output[0];
    mmal_format_copy(encoder_output->format, encoder_input->format);

    encoder_output->format->encoding = MMAL_ENCODING_H264;
    encoder_output->format->bitrate = MAX_BITRATE_LEVEL4;
    encoder_output->buffer_size = encoder_output->buffer_size_recommended;
    if (encoder_output->buffer_size < encoder_output->buffer_size_min) {
        encoder_output->buffer_size = encoder_output->buffer_size_min;
    }
    encoder_output->buffer_num = encoder_output->buffer_num_recommended;
    if (encoder_output->buffer_num < encoder_output->buffer_num_min) {
        encoder_output->buffer_num = encoder_output->buffer_num_min;
    }

    // Set frame rate for now to 0, update it later upon port connection
    encoder_output->format->es->video.frame_rate.num = 0;
    encoder_output->format->es->video.frame_rate.den = 1;

    cout << "Setting encoder output format" << endl;
    status = mmal_port_format_commit(encoder_output);
    ABORT_IF_ERROR(status);

    // Set Intraperiod (how many I frames per P frame) and Quantization Parameters. Does this affect motion vectors?
    // Skip for now since we are dumping data

    MMAL_PARAMETER_VIDEO_PROFILE_T param;
    param.hdr.id = MMAL_PARAMETER_PROFILE;
    param.hdr.size = sizeof(param);
    param.profile[0].profile = MMAL_VIDEO_PROFILE_H264_BASELINE;
    if ((VCOS_ALIGN_UP(width, 16) >> 4) * (VCOS_ALIGN_UP(height, 16) >> 4) * framerate > 245760) {
        if ((VCOS_ALIGN_UP(width, 16) >> 4) * (VCOS_ALIGN_UP(height, 16) >> 4) * framerate <= 522240) {
            cout << "Too many macroblocks per second. Increase H264 to level 4.2" << endl;
            level = MMAL_VIDEO_LEVEL_H264_42;
        } else {
            cout << "ERROR: Too many macroblocks per second. Need new resolution and framerate settings!" << endl;
            exit(-1);
        }
    }
    param.profile[0].level = level;

    cout << "Setting encoder profile" << endl;
    status = mmal_port_parameter_set(encoder_output, &param.hdr);
    ABORT_IF_ERROR(status);

    // Might change for OPQV
    if (mmal_port_parameter_set_boolean(encoder_input, MMAL_PARAMETER_VIDEO_IMMUTABLE_INPUT, 1) != MMAL_SUCCESS) {
        cout << "Could not set immutable input flag" << endl;
    }
    // Dump headers. don't need them
    if (mmal_port_parameter_set_boolean(encoder_output, MMAL_PARAMETER_VIDEO_ENCODE_INLINE_HEADER, false) != MMAL_SUCCESS) {
        cout << "Could not disable inline headers" << endl;
    }
    // NEED inline motion vectors!
    if (mmal_port_parameter_set_boolean(encoder_output, MMAL_PARAMETER_VIDEO_ENCODE_INLINE_VECTORS, 1) != MMAL_SUCCESS) {
        cout << "ERROR: Could not enable inline motion vectors!" << endl;
        exit(-1);
    }

    // Skip adaptive intra motion refresh
    
    cout << "Enable encoder" << endl;
    status = mmal_component_enable(encoder);
    ABORT_IF_ERROR(status);

    pool = mmal_port_pool_create(encoder_output, encoder_output->buffer_num, encoder_output->buffer_size);
    if (!pool) {
        cout << "Couldn't create output buffer pool for encoder" << endl;
    }
    encoder_pool = pool;

    cout << "Encoder created" << endl;

}

int main(int argc, char** argv) {
    bcm_host_init();
    init_camera();
    init_splitter();
    init_encoder();
    cleanup();
}
