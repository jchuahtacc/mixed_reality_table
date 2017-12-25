#include "RaspiVid.h"
#include <memory>

using namespace std;
using namespace raspivid;

class FrameCallback : public RaspiCallback {
    public:
        char *frame_buffer;
        int width_, height_, size_;

        // Initialize a frame buffer to contain a vcos_aligned width and height sized frame
        FrameCallback(int width, int height) : width_(VCOS_ALIGN_UP(width, 32)), height_(VCOS_ALIGN_UP(height, 16)), size_(width_ * height_) {
            frame_buffer = (char*)malloc(size_);
        }

        ~FrameCallback() {
            free(frame_buffer);
        }

        // override callback function
        void callback(MMAL_PORT_T* port, MMAL_BUFFER_HEADER_T* buffer) {
            vcos_log_error("Buffer length %d", buffer->length);
            if (buffer->length >= size_) {
                vcos_log_error("Copying grayscale data to frame buffer");
                // Copy buffer->data while the buffer is locked
                // Since the data is YUV, we are only copying the grayscale Y plane 
                memcpy(frame_buffer, buffer->data, size_);
            }
        }

        void post_process() {
            // Do any post processing to our own copy of the data after we have released the buffer
            vcos_log_error("Post processing frame buffer");
        }
};

class MotionVectorCallback : public RaspiCallback {
    public: 
        int width_, height_;
        // Make an overlay renderer that we can use to show motion data
        shared_ptr< RaspiOverlayRenderer > overlay_renderer = RaspiOverlayRenderer::create();

        MotionVectorCallback(int width, int height) : width_(VCOS_ALIGN_UP(width, 32)), height_(VCOS_ALIGN_UP(height, 16)) {
        }

        // Get the MSB of the sum of absolute differences for a given macroblock
        int buffer_offset(int row, int col) {
            return (row * (width_ / 16 + 1) + col) * 4 + 2;
        }

        void callback(MMAL_PORT_T* port, MMAL_BUFFER_HEADER_T* buffer) {
            if (buffer->flags & MMAL_BUFFER_HEADER_FLAG_CODECSIDEINFO) {
                vcos_log_error("Got motion vectors");
                // Get an RGB buffer from the overlay renderer
                MMAL_BUFFER_HEADER_T *overlay = overlay_renderer->get_buffer();
                // Fill the overlay buffer's red channel with values from the motion vectors
                for (int row = 0; row < height_; row++) {
                    for (int col = 0; col < width_; col++) {
                        overlay->data[((row * width_) + col) * 3] = buffer->data[buffer_offset(row / 16, col / 16)];    
                    }
                }
                // Send the filled overlay buffer back to the renderer
                overlay_renderer->send_buffer(overlay);
            }
        }
};

// Create shared_ptrs for each component
shared_ptr< RaspiCamera > camera = RaspiCamera::create();
shared_ptr< RaspiRenderer > preview_renderer = RaspiRenderer::create();
shared_ptr< RaspiNullsink > nullsink = RaspiNullsink::create();
shared_ptr< RaspiSplitter > splitter = RaspiSplitter::create();
shared_ptr< RaspiEncoder > encoder = RaspiEncoder::create();
shared_ptr< FrameCallback > frameCallbackPtr;
shared_ptr< MotionVectorCallback > motionVectorCallbackPtr;

// Initialize all components
MMAL_STATUS_T init() {
    MMAL_STATUS_T status;

    // Connect camera preview to preview renderer
    if ((status = preview_renderer->input->connect(camera->preview)) != MMAL_SUCCESS) {
        vcos_log_error("Couldn't connect camera preview to preview renderer");
        return status;
    }

    // Need camera video encoding to be I420 to be able to get raw frame output
    RASPIPORT_FORMAT_S format = camera->video->get_format();
    format.encoding = MMAL_ENCODING_I420;
    if ((status = camera->video->set_format(format)) != MMAL_SUCCESS) {
        vcos_log_error("Couldn't set camera video format to YUV");
    }

    int width = format.width;
    int height = format.height;

    // Connect camera video to splitter
    if ((status = splitter->input->connect(camera->video)) != MMAL_SUCCESS) {
        vcos_log_error("Couldn't connect camera preview to preview renderer");
        return status;
    }
    // After we connect the splitter, we have to explicitly duplicate the input port format to both outputs
    splitter->duplicate_input();

    // Connect encoder to splitter
    if ((status = encoder->input->connect(splitter->output_1)) != MMAL_SUCCESS) {
        vcos_log_error("Couldn't connect camera preview to preview renderer");
        return status;
    }

    // Create shared_ptr callback instances
    frameCallbackPtr = shared_ptr< FrameCallback >(new FrameCallback(width,  height));
    motionVectorCallbackPtr = shared_ptr< MotionVectorCallback >(new MotionVectorCallback(width, height));

    // Attach callbacks
    if ((status = splitter->output_0->add_callback(frameCallbackPtr)) != MMAL_SUCCESS) {
        vcos_log_error("Couldn't add frame buffer callback to splitter output 0");
        return status;
    }
    if ((status = encoder->output->add_callback(motionVectorCallbackPtr)) != MMAL_SUCCESS) {
        vcos_log_error("Couldn't add motion vector callback to encoder output");
        return status;
    }

    return MMAL_SUCCESS;
}

// Wait for Ctrl-C
int wait() {
    sigset_t waitset;
    int sig;
    int result = 0;

    sigemptyset( &waitset );
    sigaddset( &waitset, SIGUSR1 );
    pthread_sigmask( SIG_BLOCK, &waitset, NULL );
    result = sigwait ( &waitset, &sig );
    return 1;
}


int main(int argc, char** argv) {
    
    if (init() != MMAL_SUCCESS) {
        vcos_log_error("One or more components failed to initialize");
        return -1;
    }

    vcos_log_error("Starting processing. Press Ctrl-C to exit...");
    if (camera->start() != MMAL_SUCCESS) {
        vcos_log_error("Camera failed to start");
        return -1;
    }
    vcos_log_error("Camera started");

    while (wait());

    vcos_log_error("Exiting...");
}
