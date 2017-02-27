#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <cstring>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <raspberrypi-omxcam/omxcam.h>
#include <chrono>

using namespace std;
using namespace std::chrono;
using namespace cv;

const int width = 640;
const int height = 480;

char* frame_buffer;


uint32_t frames = 0;
uint32_t frame_size;
omxcam_yuv_planes_t planes;
omxcam_yuv_planes_t planes_slice;
uint32_t offset_y;
uint32_t offset_u;
uint32_t offset_v;

cv::Mat frame(height, width, 0);

int fd;
int fd_motion;
uint32_t current = 0;
int timeout = 2000;
int quit = 0;

int log_error (){
  omxcam_perror ();
  return 1;
}

void signal_handler (int signal){
  quit = 1;
}

void start_timer (){
  signal (SIGALRM, signal_handler);
  
  struct itimerval timer;
  
  timer.it_value.tv_sec = timeout/1000;
  timer.it_value.tv_usec = (timeout*1000)%1000000;
  timer.it_interval.tv_sec = timer.it_interval.tv_usec = 0;
  
  setitimer (ITIMER_REAL, &timer, 0);
}

int save (char* filename, char* motion, omxcam_video_settings_t* settings){
  printf ("capturing %s\n", filename);
  printf ("capturing motion %s\n", motion);
  
  fd = open (filename, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0666);
  if (fd == -1){
    fprintf (stderr, "error: open\n");
    return 1;
  }
  
  fd_motion = open (motion, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0666);
  if (fd_motion == -1){
    fprintf (stderr, "error: open (motion)\n");
    return 1;
  }
  
  if (omxcam_video_start_npt (settings)) return log_error ();
  omxcam_buffer_t buffer;
  omxcam_bool is_motion_vector;

  // motion vector data is 4 byte sequences, 1 for each macroblock. (Extra column of macroblocks)
  // 2nd half is a sum of absolute differences. (High SAD means change)
  //
  // TODO: Try to write h264 data to named FIFO file, read FIFO with OpenCV Video Capture

  start_timer ();
  int frames = 0;
  long duration = 0;
  steady_clock::time_point frameStart = steady_clock::now();
  while (!quit){
    //When read() is called, the current thread is locked until 'on_data' is
    //executed or an error occurs
    if (omxcam_video_read_npt (&buffer, &is_motion_vector)) return log_error ();
    
    if (is_motion_vector){
      if (pwrite (fd_motion, buffer.data, buffer.length, 0) == -1){
        fprintf (stderr, "error: pwrite (motion)\n");
        if (omxcam_video_stop_npt ()) log_error ();
      }
    }else{
      current += buffer.length;

      //Append only the Y planar data to the buffer
      memcpy (frame_buffer + offset_y, buffer.data + planes_slice.offset_y, planes_slice.length_y);
      offset_y += planes_slice.length_y;
      /*
      memcpy (file_buffer + offset_u, buffer.data + planes_slice.offset_u, planes_slice.length_u);
      offset_u += planes_slice.length_u;
      memcpy (file_buffer + offset_v, buffer.data + planes_slice.offset_v, planes_slice.length_v);
      offset_v += planes_slice.length_v;
      */
      if (current == frame_size){
        //An entire YUV frame has been received
        current = 0;
      
        offset_y = 0;
        frame = cv::Mat(height, width, 0, frame_buffer);
        frames++;
        if (frames > 10) {
            quit = true;
        }
      }
    }
  }
  
        steady_clock::time_point frameEnd = steady_clock::now();
        duration = duration_cast<milliseconds>(frameEnd - frameStart).count();
    cout << "Duration: " << duration << endl;
  if (omxcam_video_stop_npt ()) return log_error ();
  
  if (close (fd)){
    fprintf (stderr, "error: close\n");
    return 1;
  }
  
  if (close (fd_motion)){
    fprintf (stderr, "error: close (motion)\n");
    return 1;
  }
  
  return 0;
}

int main (){
  omxcam_video_settings_t settings;
  
  omxcam_video_init (&settings);
  settings.camera.width = width;
  settings.camera.height = height;
  settings.format = OMXCAM_FORMAT_YUV420;
  omxcam_yuv_planes (width, height, &planes);
  omxcam_yuv_planes_slice (width, &planes_slice);

  frame_size = planes.offset_v + planes.length_v;

  cout << "Frame size: " << frame_size<< endl;
  int plane_size = planes.length_y;
  cout << "Y plane size: " << planes.length_y << endl;

  frame_buffer = new char[planes.length_y];
          
  offset_y = planes.offset_y;
  offset_u = planes.offset_u;
  offset_v = planes.offset_v;

  if (save ("video.h264", "motion", &settings)) return 1;
  
  printf ("ok\n");
  
  return 0;
}
