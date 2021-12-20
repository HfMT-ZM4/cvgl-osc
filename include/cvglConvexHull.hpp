#pragma once

#include "opencv2/core/utility.hpp"
#include "opencv2/imgproc.hpp"

namespace cvgl
{
    enum { CALIPERS_MAXHEIGHT=0, CALIPERS_MINAREARECT=1, CALIPERS_MAXDIST=2 };
    void rotatingCalipers( const cv::Point2f* points, int n, int mode, float* out );
    
    void convexHull( cv::InputArray _points, cv::OutputArray _hull, bool clockwise, bool returnPoints );

    void convexHullPI( cv::InputArray _points, cv::OutputArray _hullP, cv::OutputArray _hullI, bool clockwise );

    void minAreaRectHull( const cv::Mat& _points, cv::RotatedRect& box, cv::Mat& hullP, cv::Mat& hullI );

    void convexityDefects( cv::InputArray _points, cv::InputArray _hull, cv::OutputArray _defects );
    bool isContourConvex( cv::InputArray _contour );

}

