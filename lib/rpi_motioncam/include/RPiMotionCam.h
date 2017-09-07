#ifndef __RPIMOTIONCAM_HPP__
#define __RPIMOTIONCAM_HPP__

namespace rpi_motioncam {
    class RPiMotionCam {
        public:
            RPiMotionCam(int width = 640, int height = 480);
            ~RPiMotionCam();
        private:
            int width_ = 640;
            int height_ = 480;
    };
}

#endif
