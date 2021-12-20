
#include "cvglCVCamera.hpp"

using namespace std;

cvglCVCamera::cvglCVCamera(int index)
{
    cap.open(index, cv::CAP_V4L2);
    if(!cap.isOpened())  // check if we succeeded
    {
        return;
    }
    
    m_opencamera = true;
    
    cap.set(cv::CAP_PROP_FPS, 30);
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 1920);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);


    m_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    m_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    
    cout << "cv cam size " << m_width << " " << m_height << endl;

}

// opencv camera callback
void cvglCVCamera::cvCamLoop()
{
    cv::UMat _frame;
    
    while( cap.isOpened() && m_stop_cv_loop == false )
    {
        if( !m_paused )
        {
            cap >> _frame;
            
            /**
                note: processFrameCallback may take ownership of Mat
             */
            if( m_processFrameCallback )
                m_processFrameCallback( _frame );
        }
        
    }
    
    cout << "cameraLoop end" << endl;
}

void cvglCVCamera::start()
{
    // start camear loop (on same thread here...)
    m_cvCam_thread = thread(&cvglCVCamera::cvCamLoop, this);

}

void cvglCVCamera::pause(bool state)
{
    m_paused = state;
}

void cvglCVCamera::stop()
{
    cout << "stop()" << endl;

    // signal cv thread stop
    m_stop_cv_loop = true;
    
    if( m_cvCam_thread.joinable() )
        m_cvCam_thread.join();
    
    // close camera after join
    if( cap.isOpened() )
        cap.release();
}

