#ifndef __MARKER_HPP__
#define __MARKER_HPP__

#include <opencv2/core/core.hpp>
#include <cmath>

namespace mrtable {
    namespace process {
        class Marker {
            public:
                int deathCounter = 0;
                Point2f cameraPos;
                double cameraRot = -1;
                bool visible = false;
                Point2f dPos;
                Point2f velocity;
                float speed;
                float lastSpeed;
                float accel = 0;

                Marker() {
                }

                ~Marker() {
                }

                void reset() {
                    deathCounter = 0;
                    cameraPos.x = 0;
                    cameraPos.y = 0;
                    cameraRot = 0;
                    visible = false;
                    dPos.x = 0;
                    dPos.y = 0;
                    velocity.x = 0;
                    velocity.y = 0;
                    speed = 0;
                    lastSpeed = 0;
                    accel = 0;
                }

                void calculate(vector< Point2f >& corners, int movementThreshold, float angleThreshold, int dt) {
                    float newX = corners[0].x + corners[2].x / 2;
                    float newY = corners[0].y + corners[2].y / 2;
                    if (abs(cameraPos.x - newX) > movementThreshold || abs(cameraPos.y - newY) > movementThreshold) { 
                        dPos.x = cameraPos.x - newX;
                        dPos.y = cameraPos.y - newY;
                        cameraPos.x = newX;
                        cameraPos.y = newY;       
                        velocity.x = dPos.x / dt;
                        velocity.y = dPos.y / dt;
                        lastSpeed = speed;
                        speed = (float)sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
                        accel = (speed - lastSpeed) / dt;
                    }
                    float midX = (corners[1].x + corners[2].x) / 2 - cameraPos.x;
                    float midY = (corners[1].y + corners[2].y) / 2 - cameraPos.y;
                    float newRot = atan2(midY, midX);
                    if (abs(newRot - cameraRot) > angleThreshold) {
                        cameraRot = newRot;
                    }
                }
        };
    }
}

#endif

