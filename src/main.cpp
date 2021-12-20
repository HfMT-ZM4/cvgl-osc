// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html

#include <iostream>
#include <iomanip>
#include <vector>

#include "opencv2/core/ocl.hpp"
#include "opencv2/core/utility.hpp"

#include "cvglCVCamera.hpp"
#include "cvglDeckLinkCamera.hpp"

#include "cvglContext.hpp"
#include "cvglObject.hpp"

using namespace std;
using namespace cv;

std::unique_ptr<cvglObject>     rect;


int main( void )
{
    cvglContext context;

    cvglCVCamera cam(0);
    cvglDeckLinkCamera bm_cam(1);

    if( bm_cam.hasCamera() )
    {
        cout << "found blackmagic 2 " << endl;
      //  bm_cam.setProcessFrameCallback( [&app](cv::Mat& mat) { app.processFrame(mat, 1); } );
        context.setupWindow( bm_cam.getWidth(), bm_cam.getHeight() );
        bm_cam.start();

    }

    cout << "hi!" << endl;

    while( !context.shouldClose() )
    {

        context.clear();

        context.drawAndPoll();

    }


    return 0;
}
