
#include "cvglConvexHull.hpp"
#include "opencv2/core/types_c.h"

/*M///////////////////////////////////////////////////////////////////////////////////////
 //
 //  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
 //
 //  By downloading, copying, installing or using the software you agree to this license.
 //  If you do not agree to this license, do not download, install,
 //  copy or use the software.
 //
 //
 //                           License Agreement
 //                For Open Source Computer Vision Library
 //
 // Copyright (C) 2000, Intel Corporation, all rights reserved.
 // Third party copyrights are property of their respective owners.
 //
 // Redistribution and use in source and binary forms, with or without modification,
 // are permitted provided that the following conditions are met:
 //
 //   * Redistribution's of source code must retain the above copyright notice,
 //     this list of conditions and the following disclaimer.
 //
 //   * Redistribution's in binary form must reproduce the above copyright notice,
 //     this list of conditions and the following disclaimer in the documentation
 //     and/or other materials provided with the distribution.
 //
 //   * The name of OpenCV Foundation may not be used to endorse or promote products
 //     derived from this software without specific prior written permission.
 //
 // This software is provided by the copyright holders and contributors "as is" and
 // any express or implied warranties, including, but not limited to, the implied
 // warranties of merchantability and fitness for a particular purpose are disclaimed.
 // In no event shall the OpenCV Foundation or contributors be liable for any direct,
 // indirect, incidental, special, exemplary, or consequential damages
 // (including, but not limited to, procurement of substitute goods or services;
 // loss of use, data, or profits; or business interruption) however caused
 // and on any theory of liability, whether in contract, strict liability,
 // or tort (including negligence or otherwise) arising in any way out of
 // the use of this software, even if advised of the possibility of such damage.
 //
 //M*/


// #include "precomp.hpp"

using namespace cv;

namespace cvgl {
    
    struct MinAreaState
    {
        int bottom;
        int left;
        float height;
        float width;
        float base_a;
        float base_b;
    };
    
    /*F///////////////////////////////////////////////////////////////////////////////////////
     //    Name:    rotatingCalipers
     //    Purpose:
     //      Rotating calipers algorithm with some applications
     //
     //    Context:
     //    Parameters:
     //      points      - convex hull vertices ( any orientation )
     //      n           - number of vertices
     //      mode        - concrete application of algorithm
     //                    can be  CV_CALIPERS_MAXDIST   or
     //                            CV_CALIPERS_MINAREARECT
     //      left, bottom, right, top - indexes of extremal points
     //      out         - output info.
     //                    In case CV_CALIPERS_MAXDIST it points to float value -
     //                    maximal height of polygon.
     //                    In case CV_CALIPERS_MINAREARECT
     //                    ((CvPoint2D32f*)out)[0] - corner
     //                    ((CvPoint2D32f*)out)[1] - vector1
     //                    ((CvPoint2D32f*)out)[2] - vector2
     //
     //                      ^
     //                      |
     //              vector2 |
     //                      |
     //                      |____________\
     //                    corner         /
     //                               vector1
     //
     //    Returns:
     //    Notes:
     //F*/
    
    /* we will use usual cartesian coordinates */
    void rotatingCalipers( const cv::Point2f* points, int n, int mode, float* out )
    {
        float minarea = FLT_MAX;
        float max_dist = 0;
        char buffer[32] = {};
        int i, k;
        cv::AutoBuffer<float> abuf(n*3);
        float* inv_vect_length = abuf.data();
        cv::Point2f* vect = (Point2f*)(inv_vect_length + n);
        int left = 0, bottom = 0, right = 0, top = 0;
        int seq[4] = { -1, -1, -1, -1 };
        
        /* rotating calipers sides will always have coordinates
         (a,b) (-b,a) (-a,-b) (b, -a)
         */
        /* this is a first base bector (a,b) initialized by (1,0) */
        float orientation = 0;
        float base_a;
        float base_b = 0;
        
        float left_x, right_x, top_y, bottom_y;
        cv::Point2f pt0 = points[0];
        
        left_x = right_x = pt0.x;
        top_y = bottom_y = pt0.y;
        
        for( i = 0; i < n; i++ )
        {
            double dx, dy;
            
            if( pt0.x < left_x )
                left_x = pt0.x, left = i;
            
            if( pt0.x > right_x )
                right_x = pt0.x, right = i;
            
            if( pt0.y > top_y )
                top_y = pt0.y, top = i;
            
            if( pt0.y < bottom_y )
                bottom_y = pt0.y, bottom = i;
            
            cv::Point2f pt = points[(i+1) & (i+1 < n ? -1 : 0)];
            
            dx = pt.x - pt0.x;
            dy = pt.y - pt0.y;
            
            vect[i].x = (float)dx;
            vect[i].y = (float)dy;
            inv_vect_length[i] = (float)(1./std::sqrt(dx*dx + dy*dy));
            
            pt0 = pt;
        }
        
        // find convex hull orientation
        {
            double ax = vect[n-1].x;
            double ay = vect[n-1].y;
            
            for( i = 0; i < n; i++ )
            {
                double bx = vect[i].x;
                double by = vect[i].y;
                
                double convexity = ax * by - ay * bx;
                
                if( convexity != 0 )
                {
                    orientation = (convexity > 0) ? 1.f : (-1.f);
                    break;
                }
                ax = bx;
                ay = by;
            }
            CV_Assert( orientation != 0 );
        }
        base_a = orientation;
        
        /*****************************************************************************************/
        /*                         init calipers position                                        */
        seq[0] = bottom;
        seq[1] = right;
        seq[2] = top;
        seq[3] = left;
        /*****************************************************************************************/
        /*                         Main loop - evaluate angles and rotate calipers               */
        
        /* all of edges will be checked while rotating calipers by 90 degrees */
        for( k = 0; k < n; k++ )
        {
            /* sinus of minimal angle */
            /*float sinus;*/
            
            /* compute cosine of angle between calipers side and polygon edge */
            /* dp - dot product */
            float dp[4] = {
                +base_a * vect[seq[0]].x + base_b * vect[seq[0]].y,
                -base_b * vect[seq[1]].x + base_a * vect[seq[1]].y,
                -base_a * vect[seq[2]].x - base_b * vect[seq[2]].y,
                +base_b * vect[seq[3]].x - base_a * vect[seq[3]].y,
            };
            
            float maxcos = dp[0] * inv_vect_length[seq[0]];
            
            /* number of calipers edges, that has minimal angle with edge */
            int main_element = 0;
            
            /* choose minimal angle */
            for ( i = 1; i < 4; ++i )
            {
                float cosalpha = dp[i] * inv_vect_length[seq[i]];
                if (cosalpha > maxcos)
                {
                    main_element = i;
                    maxcos = cosalpha;
                }
            }
            
            /*rotate calipers*/
            {
                //get next base
                int pindex = seq[main_element];
                float lead_x = vect[pindex].x*inv_vect_length[pindex];
                float lead_y = vect[pindex].y*inv_vect_length[pindex];
                switch( main_element )
                {
                    case 0:
                        base_a = lead_x;
                        base_b = lead_y;
                        break;
                    case 1:
                        base_a = lead_y;
                        base_b = -lead_x;
                        break;
                    case 2:
                        base_a = -lead_x;
                        base_b = -lead_y;
                        break;
                    case 3:
                        base_a = -lead_y;
                        base_b = lead_x;
                        break;
                    default:
                        printf( "error : main_element should be 0, 1, 2 or 3" );
                }
            }
            /* change base point of main edge */
            seq[main_element] += 1;
            seq[main_element] = (seq[main_element] == n) ? 0 : seq[main_element];
            
            switch (mode)
            {
                case CALIPERS_MAXHEIGHT:
                {
                    /* now main element lies on edge aligned to calipers side */
                    
                    /* find opposite element i.e. transform  */
                    /* 0->2, 1->3, 2->0, 3->1                */
                    int opposite_el = main_element ^ 2;
                    
                    float dx = points[seq[opposite_el]].x - points[seq[main_element]].x;
                    float dy = points[seq[opposite_el]].y - points[seq[main_element]].y;
                    float dist;
                    
                    if( main_element & 1 )
                        dist = (float)fabs(dx * base_a + dy * base_b);
                    else
                        dist = (float)fabs(dx * (-base_b) + dy * base_a);
                    
                    if( dist > max_dist )
                        max_dist = dist;
                }
                    break;
                case CALIPERS_MINAREARECT:
                    /* find area of rectangle */
                {
                    float height;
                    float area;
                    
                    /* find vector left-right */
                    float dx = points[seq[1]].x - points[seq[3]].x;
                    float dy = points[seq[1]].y - points[seq[3]].y;
                    
                    /* dotproduct */
                    float width = dx * base_a + dy * base_b;
                    
                    /* find vector left-right */
                    dx = points[seq[2]].x - points[seq[0]].x;
                    dy = points[seq[2]].y - points[seq[0]].y;
                    
                    /* dotproduct */
                    height = -dx * base_b + dy * base_a;
                    
                    area = width * height;
                    if( area <= minarea )
                    {
                        float *buf = (float *) buffer;
                        
                        minarea = area;
                        /* leftist point */
                        ((int *) buf)[0] = seq[3];
                        buf[1] = base_a;
                        buf[2] = width;
                        buf[3] = base_b;
                        buf[4] = height;
                        /* bottom point */
                        ((int *) buf)[5] = seq[0];
                        buf[6] = area;
                    }
                }
                    break;
            }                       /*switch */
        }                           /* for */
        
        switch (mode)
        {
            case CALIPERS_MINAREARECT:
            {
                float *buf = (float *) buffer;
                
                float A1 = buf[1];
                float B1 = buf[3];
                
                float A2 = -buf[3];
                float B2 = buf[1];
                
                float C1 = A1 * points[((int *) buf)[0]].x + points[((int *) buf)[0]].y * B1;
                float C2 = A2 * points[((int *) buf)[5]].x + points[((int *) buf)[5]].y * B2;
                
                float idet = 1.f / (A1 * B2 - A2 * B1);
                
                float px = (C1 * B2 - C2 * B1) * idet;
                float py = (A1 * C2 - A2 * C1) * idet;
                
                out[0] = px;
                out[1] = py;
                
                out[2] = A1 * buf[2];
                out[3] = B1 * buf[2];
                
                out[4] = A2 * buf[4];
                out[5] = B2 * buf[4];
            }
                break;
            case CALIPERS_MAXHEIGHT:
            {
                out[0] = max_dist;
            }
                break;
        }
    }
    
    
    void minAreaRectHull( const Mat& _points, RotatedRect& box, Mat& hullP, Mat& hullI )
    {
        
        // to do optimizations:
        /*
         1. use Mats instead of vectors since that's how this functions works
         
         2. do one call to convectHull with an integer array, (or set false?) to get the point indexes from the contour
         3. convert the hull uses below to be lookup in the contour via the index numbers
         
         2.3... actully probably make a fork of convexhull that does both integer and points without running the function twice
         the problem is that the cvgl_rotatingCalipers uses the points via a pointer so either way, we have to make a vector (or array) of points
         so, easiest will be to do both in the convex hull function...
         
         
         */
        
        cv::Point2f out[3];
        cvgl::convexHullPI( _points, hullP, hullI, true );
        
        if( hullP.depth() != CV_32F )
        {
            cv::Mat temp;
            hullP.convertTo(temp, CV_32F);
            hullP = temp;
        }
        
        int n = hullP.checkVector(2);
        const cv::Point2f* hpoints = hullP.ptr<cv::Point2f>();
        
        
        if( n > 2 )
        {
            cvgl::rotatingCalipers( hpoints, n, CALIPERS_MINAREARECT, (float*)out );
            box.center.x = out[0].x + (out[1].x + out[2].x)*0.5f;
            box.center.y = out[0].y + (out[1].y + out[2].y)*0.5f;
            box.size.width = (float)std::sqrt((double)out[1].x*out[1].x + (double)out[1].y*out[1].y);
            box.size.height = (float)std::sqrt((double)out[2].x*out[2].x + (double)out[2].y*out[2].y);
            box.angle = (float)atan2( (double)out[1].y, (double)out[1].x );
        }
        else if( n == 2 )
        {
            box.center.x = (hpoints[0].x + hpoints[1].x)*0.5f;
            box.center.y = (hpoints[0].y + hpoints[1].y)*0.5f;
            double dx = hpoints[1].x - hpoints[0].x;
            double dy = hpoints[1].y - hpoints[0].y;
            box.size.width = (float)std::sqrt(dx*dx + dy*dy);
            box.size.height = 0;
            box.angle = (float)atan2( dy, dx );
        }
        else
        {
            if( n == 1 )
                box.center = hpoints[0];
        }
        
        box.angle = (float)(box.angle*180/CV_PI);
    }
  
    template<typename _Tp>
    int Sklansky_( cv::Point_<_Tp>** array, int start, int end, int* stack, int nsign, int sign2 )
    {
        int incr = end > start ? 1 : -1;
        // prepare first triangle
        int pprev = start, pcur = pprev + incr, pnext = pcur + incr;
        int stacksize = 3;
        
        if( start == end ||
           (array[start]->x == array[end]->x &&
            array[start]->y == array[end]->y) )
        {
            stack[0] = start;
            return 1;
        }
        
        stack[0] = pprev;
        stack[1] = pcur;
        stack[2] = pnext;
        
        end += incr; // make end = afterend
        
        while( pnext != end )
        {
            // check the angle p1,p2,p3
            _Tp cury = array[pcur]->y;
            _Tp nexty = array[pnext]->y;
            _Tp by = nexty - cury;
            
            if( CV_SIGN( by ) != nsign )
            {
                _Tp ax = array[pcur]->x - array[pprev]->x;
                _Tp bx = array[pnext]->x - array[pcur]->x;
                _Tp ay = cury - array[pprev]->y;
                _Tp convexity = ay*bx - ax*by; // if >0 then convex angle
                
                if( CV_SIGN( convexity ) == sign2 && (ax != 0 || ay != 0) )
                {
                    pprev = pcur;
                    pcur = pnext;
                    pnext += incr;
                    stack[stacksize] = pnext;
                    stacksize++;
                }
                else
                {
                    if( pprev == start )
                    {
                        pcur = pnext;
                        stack[1] = pcur;
                        pnext += incr;
                        stack[2] = pnext;
                    }
                    else
                    {
                        stack[stacksize-2] = pnext;
                        pcur = pprev;
                        pprev = stack[stacksize-4];
                        stacksize--;
                    }
                }
            }
            else
            {
                pnext += incr;
                stack[stacksize-1] = pnext;
            }
        }
        
        return --stacksize;
    }
    
    
    template<typename _Tp>
    struct CHullCmpPoints
    {
        bool operator()(const cv::Point_<_Tp>* p1, const cv::Point_<_Tp>* p2) const
        { return p1->x < p2->x || (p1->x == p2->x && p1->y < p2->y); }
    };
    
    
    void convexHull( cv::InputArray _points, cv::OutputArray _hull, bool clockwise, bool returnPoints )
    {
        
        CV_Assert(_points.getObj() != _hull.getObj());
        Mat points = _points.getMat();
        int i, total = points.checkVector(2), depth = points.depth(), nout = 0;
        int miny_ind = 0, maxy_ind = 0;
        CV_Assert(total >= 0 && (depth == CV_32F || depth == CV_32S));
        
        if( total == 0 )
        {
            _hull.release();
            return;
        }
        
        returnPoints = !_hull.fixedType() ? returnPoints : _hull.type() != CV_32S;
        
        bool is_float = depth == CV_32F;
        AutoBuffer<Point*> _pointer(total);
        AutoBuffer<int> _stack(total + 2), _hullbuf(total);
        Point** pointer = _pointer.data();
        Point2f** pointerf = (Point2f**)pointer;
        Point* data0 = points.ptr<Point>();
        int* stack = _stack.data();
        int* hullbuf = _hullbuf.data();
        
        CV_Assert(points.isContinuous());
        
        for( i = 0; i < total; i++ )
            pointer[i] = &data0[i];
        
        // sort the point set by x-coordinate, find min and max y
        if( !is_float )
        {
            std::sort(pointer, pointer + total, CHullCmpPoints<int>());
            for( i = 1; i < total; i++ )
            {
                int y = pointer[i]->y;
                if( pointer[miny_ind]->y > y )
                    miny_ind = i;
                if( pointer[maxy_ind]->y < y )
                    maxy_ind = i;
            }
        }
        else
        {
            std::sort(pointerf, pointerf + total, CHullCmpPoints<float>());
            for( i = 1; i < total; i++ )
            {
                float y = pointerf[i]->y;
                if( pointerf[miny_ind]->y > y )
                    miny_ind = i;
                if( pointerf[maxy_ind]->y < y )
                    maxy_ind = i;
            }
        }
        
        if( pointer[0]->x == pointer[total-1]->x &&
           pointer[0]->y == pointer[total-1]->y )
        {
            hullbuf[nout++] = 0;
        }
        else
        {
            // upper half
            int *tl_stack = stack;
            int tl_count = !is_float ?
            Sklansky_( pointer, 0, maxy_ind, tl_stack, -1, 1) :
            Sklansky_( pointerf, 0, maxy_ind, tl_stack, -1, 1);
            int *tr_stack = stack + tl_count;
            int tr_count = !is_float ?
            Sklansky_( pointer, total-1, maxy_ind, tr_stack, -1, -1) :
            Sklansky_( pointerf, total-1, maxy_ind, tr_stack, -1, -1);
            
            // gather upper part of convex hull to output
            if( !clockwise )
            {
                std::swap( tl_stack, tr_stack );
                std::swap( tl_count, tr_count );
            }
            
            for( i = 0; i < tl_count-1; i++ )
                hullbuf[nout++] = int(pointer[tl_stack[i]] - data0);
            for( i = tr_count - 1; i > 0; i-- )
                hullbuf[nout++] = int(pointer[tr_stack[i]] - data0);
            int stop_idx = tr_count > 2 ? tr_stack[1] : tl_count > 2 ? tl_stack[tl_count - 2] : -1;
            
            // lower half
            int *bl_stack = stack;
            int bl_count = !is_float ?
            Sklansky_( pointer, 0, miny_ind, bl_stack, 1, -1) :
            Sklansky_( pointerf, 0, miny_ind, bl_stack, 1, -1);
            int *br_stack = stack + bl_count;
            int br_count = !is_float ?
            Sklansky_( pointer, total-1, miny_ind, br_stack, 1, 1) :
            Sklansky_( pointerf, total-1, miny_ind, br_stack, 1, 1);
            
            if( clockwise )
            {
                std::swap( bl_stack, br_stack );
                std::swap( bl_count, br_count );
            }
            
            if( stop_idx >= 0 )
            {
                int check_idx = bl_count > 2 ? bl_stack[1] :
                bl_count + br_count > 2 ? br_stack[2-bl_count] : -1;
                if( check_idx == stop_idx || (check_idx >= 0 &&
                                              pointer[check_idx]->x == pointer[stop_idx]->x &&
                                              pointer[check_idx]->y == pointer[stop_idx]->y) )
                {
                    // if all the points lie on the same line, then
                    // the bottom part of the convex hull is the mirrored top part
                    // (except the exteme points).
                    bl_count = MIN( bl_count, 2 );
                    br_count = MIN( br_count, 2 );
                }
            }
            
            for( i = 0; i < bl_count-1; i++ )
                hullbuf[nout++] = int(pointer[bl_stack[i]] - data0);
            for( i = br_count-1; i > 0; i-- )
                hullbuf[nout++] = int(pointer[br_stack[i]] - data0);
        }
        
        if( !returnPoints )
            Mat(nout, 1, CV_32S, hullbuf).copyTo(_hull);
        else
        {
            _hull.create(nout, 1, CV_MAKETYPE(depth, 2));
            Mat hull = _hull.getMat();
            size_t step = !hull.isContinuous() ? hull.step[0] : sizeof(Point);
            for( i = 0; i < nout; i++ )
                *(Point*)(hull.ptr() + i*step) = data0[hullbuf[i]];
        }
    }
    
    void convexHullPI( InputArray _points, OutputArray _hullP, OutputArray _hullI, bool clockwise )
    {
        
        CV_Assert(_points.getObj() != _hullP.getObj());
        Mat points = _points.getMat();
        int i, total = points.checkVector(2), depth = points.depth(), nout = 0;
        int miny_ind = 0, maxy_ind = 0;
        CV_Assert(total >= 0 && (depth == CV_32F || depth == CV_32S));
        
        if( total == 0 )
        {
            _hullP.release();
            _hullI.release();
            return;
        }
        
        //returnPoints = !_hull.fixedType() ? returnPoints : _hull.type() != CV_32S;
        
        bool is_float = depth == CV_32F;
        AutoBuffer<Point*> _pointer(total);
        AutoBuffer<int> _stack(total + 2), _hullbuf(total);
        Point** pointer = _pointer.data();
        Point2f** pointerf = (Point2f**)pointer;
        Point* data0 = points.ptr<Point>();
        int* stack = _stack.data();
        int* hullbuf = _hullbuf.data();
        
        CV_Assert(points.isContinuous());
        
        for( i = 0; i < total; i++ )
            pointer[i] = &data0[i];
            
        // sort the point set by x-coordinate, find min and max y
        if( !is_float )
        {
            std::sort(pointer, pointer + total, CHullCmpPoints<int>());
            for( i = 1; i < total; i++ )
            {
                int y = pointer[i]->y;
                if( pointer[miny_ind]->y > y )
                    miny_ind = i;

                if( pointer[maxy_ind]->y < y )
                    maxy_ind = i;
            }
        }
        else
        {
            std::sort(pointerf, pointerf + total, CHullCmpPoints<float>());
            for( i = 1; i < total; i++ )
            {
                float y = pointerf[i]->y;
                if( pointerf[miny_ind]->y > y )
                    miny_ind = i;

                if( pointerf[maxy_ind]->y < y )
                    maxy_ind = i;
            }
        }
        
        if( pointer[0]->x == pointer[total-1]->x &&
           pointer[0]->y == pointer[total-1]->y )
        {
            hullbuf[nout++] = 0;
        }
        else
        {
            // upper half
            int *tl_stack = stack;
            int tl_count = !is_float ?
            Sklansky_( pointer, 0, maxy_ind, tl_stack, -1, 1) :
            Sklansky_( pointerf, 0, maxy_ind, tl_stack, -1, 1);
            int *tr_stack = stack + tl_count;
            int tr_count = !is_float ?
            Sklansky_( pointer, total-1, maxy_ind, tr_stack, -1, -1) :
            Sklansky_( pointerf, total-1, maxy_ind, tr_stack, -1, -1);
            
            // gather upper part of convex hull to output
            if( !clockwise )
            {
                std::swap( tl_stack, tr_stack );
                std::swap( tl_count, tr_count );
            }
            
            for( i = 0; i < tl_count-1; i++ )
                hullbuf[nout++] = int(pointer[tl_stack[i]] - data0);
            
            for( i = tr_count - 1; i > 0; i-- )
                hullbuf[nout++] = int(pointer[tr_stack[i]] - data0);

            int stop_idx = tr_count > 2 ? tr_stack[1] : tl_count > 2 ? tl_stack[tl_count - 2] : -1;

            // lower half
            int *bl_stack = stack;
            int bl_count = !is_float ?
            Sklansky_( pointer, 0, miny_ind, bl_stack, 1, -1) :
            Sklansky_( pointerf, 0, miny_ind, bl_stack, 1, -1);
            int *br_stack = stack + bl_count;
            int br_count = !is_float ?
            Sklansky_( pointer, total-1, miny_ind, br_stack, 1, 1) :
            Sklansky_( pointerf, total-1, miny_ind, br_stack, 1, 1);

            if( clockwise )
            {
                std::swap( bl_stack, br_stack );
                std::swap( bl_count, br_count );
            }

            if( stop_idx >= 0 )
            {
                int check_idx = bl_count > 2 ? bl_stack[1] :
                bl_count + br_count > 2 ? br_stack[2-bl_count] : -1;
                if( check_idx == stop_idx || (check_idx >= 0 &&
                                              pointer[check_idx]->x == pointer[stop_idx]->x &&
                                              pointer[check_idx]->y == pointer[stop_idx]->y) )
                {
                    // if all the points lie on the same line, then
                    // the bottom part of the convex hull is the mirrored top part
                    // (except the exteme points).
                    bl_count = MIN( bl_count, 2 );
                    br_count = MIN( br_count, 2 );
                }
            }
            
            for( i = 0; i < bl_count-1; i++ )
                hullbuf[nout++] = int(pointer[bl_stack[i]] - data0);
                for( i = br_count-1; i > 0; i-- )
                    hullbuf[nout++] = int(pointer[br_stack[i]] - data0);
            
        }
        
        // copy points
        Mat(nout, 1, CV_32S, hullbuf).copyTo(_hullI);

        // copy Index
        _hullP.create(nout, 1, CV_MAKETYPE(depth, 2));
        Mat hull = _hullP.getMat();
        size_t step = !hull.isContinuous() ? hull.step[0] : sizeof(Point);
        for( i = 0; i < nout; i++ )
            *(Point*)(hull.ptr() + i*step) = data0[hullbuf[i]];
       
    }
    
    void convexityDefects( InputArray _points, InputArray _hull, OutputArray _defects )
    {
        
        Mat points = _points.getMat();
        int i, j = 0, npoints = points.checkVector(2, CV_32S);
        CV_Assert( npoints >= 0 );
        
        if( npoints <= 3 )
        {
            _defects.release();
            return;
        }
        
        Mat hull = _hull.getMat();
        int hpoints = hull.checkVector(1, CV_32S);
        CV_Assert( hpoints > 0 );
        
        const Point* ptr = points.ptr<Point>();
        const int* hptr = hull.ptr<int>();
        std::vector<Vec4i> defects;
        if ( hpoints < 3 ) //if hull consists of one or two points, contour is always convex
        {
            _defects.release();
            return;
        }
        
        // 1. recognize co-orientation of the contour and its hull
        bool rev_orientation = ((hptr[1] > hptr[0]) + (hptr[2] > hptr[1]) + (hptr[0] > hptr[2])) != 2;
        
        // 2. cycle through points and hull, compute defects
        int hcurr = hptr[rev_orientation ? 0 : hpoints-1];
        CV_Assert( 0 <= hcurr && hcurr < npoints );
        
        for( i = 0; i < hpoints; i++ )
        {
            int hnext = hptr[rev_orientation ? hpoints - i - 1 : i];
            CV_Assert( 0 <= hnext && hnext < npoints );
            
            Point pt0 = ptr[hcurr], pt1 = ptr[hnext];
            double dx0 = pt1.x - pt0.x;
            double dy0 = pt1.y - pt0.y;
            double scale = dx0 == 0 && dy0 == 0 ? 0. : 1./std::sqrt(dx0*dx0 + dy0*dy0);
            
            int defect_deepest_point = -1;
            double defect_depth = 0;
            bool is_defect = false;
            j=hcurr;
            for(;;)
            {
                // go through points to achieve next hull point
                j++;
                j &= j >= npoints ? 0 : -1;
                if( j == hnext )
                    break;
                
                // compute distance from current point to hull edge
                double dx = ptr[j].x - pt0.x;
                double dy = ptr[j].y - pt0.y;
                double dist = fabs(-dy0*dx + dx0*dy) * scale;
                
                if( dist > defect_depth )
                {
                    defect_depth = dist;
                    defect_deepest_point = j;
                    is_defect = true;
                }
            }
            
            if( is_defect )
            {
                int idepth = cvRound(defect_depth*256);
                defects.push_back(Vec4i(hcurr, hnext, defect_deepest_point, idepth));
            }
            
            hcurr = hnext;
        }
        
        Mat(defects).copyTo(_defects);
    }
    
    
    template<typename _Tp>
    static bool isContourConvex_( const Point_<_Tp>* p, int n )
    {
        Point_<_Tp> prev_pt = p[(n-2+n) % n];
        Point_<_Tp> cur_pt = p[n-1];
        
        _Tp dx0 = cur_pt.x - prev_pt.x;
        _Tp dy0 = cur_pt.y - prev_pt.y;
        int orientation = 0;
        
        for( int i = 0; i < n; i++ )
        {
            _Tp dxdy0, dydx0;
            _Tp dx, dy;
            
            prev_pt = cur_pt;
            cur_pt = p[i];
            
            dx = cur_pt.x - prev_pt.x;
            dy = cur_pt.y - prev_pt.y;
            dxdy0 = dx * dy0;
            dydx0 = dy * dx0;
            
            // find orientation
            // orient = -dy0 * dx + dx0 * dy;
            // orientation |= (orient > 0) ? 1 : 2;
            orientation |= (dydx0 > dxdy0) ? 1 : ((dydx0 < dxdy0) ? 2 : 3);
            if( orientation == 3 )
                return false;
            
            dx0 = dx;
            dy0 = dy;
        }
        
        return true;
    }
    
    
    bool isContourConvex( InputArray _contour )
    {
        Mat contour = _contour.getMat();
        int total = contour.checkVector(2), depth = contour.depth();
        CV_Assert(total >= 0 && (depth == CV_32F || depth == CV_32S));
        
        if( total == 0 )
            return false;
        
        return depth == CV_32S ?
        isContourConvex_(contour.ptr<Point>(), total ) :
        isContourConvex_(contour.ptr<Point2f>(), total );
    }
    
}

