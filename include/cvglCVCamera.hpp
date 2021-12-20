
#pragma once

#include <iostream>
#include <thread>

#include "opencv2/highgui.hpp"
#include "opencv2/core/utility.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"

#include "cvglCameraBase.hpp"

class cvglCVCamera : public cvglCameraBase
{
public:
    cvglCVCamera(int index = 0);
    ~cvglCVCamera()
    {
        stop();
    }
    
    void cvCamLoop();
    
    void start();
    void pause(bool state);
    void stop();
    
    
private:
    
    cv::VideoCapture cap;
    std::thread m_cvCam_thread;
    
    bool m_stop_cv_loop = false;
    
    bool m_paused = false;
    
    std::mutex m_mutex;
    
    
};
