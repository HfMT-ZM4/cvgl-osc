#include "opencv2/cudaoptflow.hpp"
#include "opencv2/cudaarithm.hpp"
#include "opencv2/video/tracking.hpp"

#include "cvglCV.hpp"
#include "cvglHelperFunctions.hpp"
//#include "cvglMainProcess.hpp"

using namespace cv;
using namespace std;
using namespace Eigen;

void cvglCV::setCVParams( MapOSC & b )
{
    unique_lock<mutex> lock(m_lock);

    for( auto& msg : b.getMap() )
    {
        const string& addr = msg.first;
        const OSCAtomVector& m = msg.second;
        if( addr == "/use/preprocess" )
        {
            int setProcessIDX = m.get<int>();
            if( setProcessIDX != m_use_preprocess )
            {
                m_use_preprocess = setProcessIDX;
                m_prev_frame.release();
                m_prev_frame = UMat();
            }

       //     cout << "setting preprocess to " << m_use_preprocess << " " << val.getInt() << endl;
        }
        else if( addr == "/invert" )
        {
            m_invert = m.getInt() > 0;
        }
        else if( addr == "/thresh" )
        {
            m_thresh = m.getFloat();
        }
        else if( addr == "/parentsonly" )
        {
            m_parents_only = m.getInt() > 0;
        }
        else if( addr == "/canny/min" )
        {
            m_canny_min = m.getFloat();
        }
        else if( addr == "/canny/max" )
        {
            m_canny_max = m.getFloat();
        }
        else if( addr == "/size/min" )
        {
            m_minsize = m.getFloat();
        }
        else if( addr == "/size/max" )
        {
            m_maxsize = m.getFloat();
        }
        else if( addr == "/gauss/sigma" )
        {
            m_gauss_sigma = m.getInt();
            m_gauss_ksize = m_gauss_sigma*5;
        }
        
    }
}

void cvglCV::preprocess()
{
   //cout << " preprocess  " << m_use_preprocess << endl;

    switch (m_use_preprocess) {
        case 0:
            preprocessBasic(); // basic image contours
            break;
        case 1:
            preprocessDifference(); // delta based movement
            break;
        case 2:
            preprocessCanny(); // edge contours
            break;
        case 3:
            preprocessDenseFlow(); // dense optical flow (better for image)
        default:
            break;
    }

}


void cvglCV::preprocessBasic()
{
    
    // later could wrap each function with a "set to output" flag, which could be used for debugging
    // or keep these all as class members and then query them as needed..
    if( m_img.empty() )
    {
        cout << "img is missing!" << endl;
        return;
    }
    
    float resize = m_resize; // avoid lock
    cv::resize(m_img, src_color_sized, cv::Size(), resize, resize, cv::INTER_AREA);
    cv::cvtColor(src_color_sized, src_gray, cv::COLOR_RGB2GRAY);
    
    // cout << "nchannels " << src_color_sized.channels() << endl;
    
    switch (m_color_mode) {
        case 1:
            cvtColor(src_color_sized, src_color_sized, COLOR_RGB2HLS_FULL);
            break;
        case 2:
            cvtColor(src_color_sized, src_color_sized, COLOR_RGB2Lab);
            break;
        default:
            break;
    }
    
    if( m_invert )
    {
        bitwise_not(src_gray, src_gray);
    }
    
    
    GaussianBlur(src_gray, src_blur_gray, cv::Size(m_gauss_ksize, m_gauss_ksize), m_gauss_sigma, m_gauss_sigma);
    erode( src_blur_gray, src_blur_gray, m_er_element );
    dilate( src_blur_gray, src_blur_gray, m_di_element );
    
    threshold( src_blur_gray, threshold_output, m_thresh, 255, cv::THRESH_BINARY  ); // + cv::THRESH_OTSU
    
    //adaptiveThreshold( src_blur_gray, threshold_output, 255,
    //                   ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 11, m_thresh);

    Sobel(src_gray, sob, CV_32F, 1, 1);
    
}


void cvglCV::preprocessDifference()
{

    if( m_img.empty() )
    {
        return;
    }


    float resize = m_resize; // avoid OSC lock?
    cv::resize(m_img, src_color_sized, cv::Size(), resize, resize, cv::INTER_AREA);
    
    if( m_prev_frame.empty() )
    {
       m_prev_frame = src_color_sized.clone();
    }

    UMat diff;
    cv::subtract( m_prev_frame, src_color_sized, diff);
       
    m_prev_frame = src_color_sized.clone();
       
    
    cv::cvtColor(diff, src_gray, cv::COLOR_RGB2GRAY);
    
    switch (m_color_mode) {
        case 1:
            cvtColor(src_color_sized, src_color_sized, COLOR_RGB2HLS_FULL);
            break;
        case 2:
            cvtColor(src_color_sized, src_color_sized, COLOR_RGB2Lab);
            break;
        default:
            break;
    }
    
    if( m_invert )
    {
        bitwise_not(src_gray, src_gray);
    }
    
    
    GaussianBlur(src_gray, src_blur_gray, cv::Size(m_gauss_ksize, m_gauss_ksize), m_gauss_sigma, m_gauss_sigma);
    erode( src_blur_gray, src_blur_gray, m_er_element );
    dilate( src_blur_gray, src_blur_gray, m_di_element );
    

    threshold( src_blur_gray, threshold_output, m_thresh, 255, cv::THRESH_BINARY );
    
    Sobel(src_gray, sob, CV_32F, 1, 1);

}

void cvglCV::preprocessCanny()
{
    
    if( m_img.empty() )
    {
        return;
    }
    
    //  m_img = mat.clone();
    
    float resize = m_resize; // avoid lock
    cv::resize(m_img, src_color_sized, cv::Size(), resize, resize, cv::INTER_AREA);
    cv::cvtColor(src_color_sized, src_gray, cv::COLOR_RGB2GRAY);
    
    switch (m_color_mode) {
        case 1:
            cvtColor(src_color_sized, src_color_sized, COLOR_RGB2HLS_FULL);
            break;
        case 2:
            cvtColor(src_color_sized, src_color_sized, COLOR_RGB2Lab);
            break;
        default:
            break;
    }
    
    if( m_invert )
    {
        bitwise_not(src_gray, src_gray);
    }
    
    
    GaussianBlur(src_gray, src_blur_gray, cv::Size(m_gauss_ksize, m_gauss_ksize), m_gauss_sigma, m_gauss_sigma);
    erode( src_blur_gray, src_blur_gray, m_er_element );
    dilate( src_blur_gray, src_blur_gray, m_di_element );
    
    Mat can;
    cv::Canny(src_blur_gray, can, m_canny_min, m_canny_max, 3);
    
    /*
    Mat can_blur;
    GaussianBlur(can, can_blur, cv::Size(m_gauss_ksize, m_gauss_ksize), m_gauss_sigma, m_gauss_sigma);
    erode( can_blur, can_blur, m_er_element );
    dilate( can_blur, can_blur, m_di_element );
    */

    // previously I was bluring the edges to make larger contours, not sure if that is still worth doing maybe

    //    src_gray = can;
    
    threshold( can, threshold_output, m_thresh, 255, cv::THRESH_BINARY );
    
    //Sobel(src_gray, sob, CV_32F, 1, 1);
    
    
    // cv::cvtColor(src_gray+can, mat, cv::COLOR_GRAY2RGB);
    
    
    
}


// quantize the image to numBits
cv::Mat quantizeImage(const cv::Mat& inImage, int numBits)
{
    cv::Mat retImage = inImage.clone();

    uchar maskBit = 0xFF;

    // keep numBits as 1 and (8 - numBits) would be all 0 towards the right
    maskBit = maskBit << (8 - numBits);

    for(int j = 0; j < retImage.rows; j++)
    {
        for(int i = 0; i < retImage.cols; i++)
        {
            cv::Vec3b valVec = retImage.at<cv::Vec3b>(j, i);
            valVec[0] = valVec[0] & maskBit;
            valVec[1] = valVec[1] & maskBit;
            valVec[2] = valVec[2] & maskBit;
            retImage.at<cv::Vec3b>(j, i) = valVec;
        }
    }

    return retImage;
}


/**
 * @brief cvglCV::colorSegmentation
 * @return vector of segemented color groups
 */
vector<UMat> cvglCV::colorSegmentation(const string& type)
{
    if( m_img.empty() )
    {
        return vector<UMat>();
    }

   // Mat q = quantizeImage(m_img.getMat(ACCESS_READ), 4);
   // q.copyTo(m_img);
    float resize = m_resize; // avoid lock
    cv::resize(m_img, src_color_sized, cv::Size(), resize, resize, cv::INTER_AREA);

    UMat filtered;
    pyrMeanShiftFiltering( src_color_sized, filtered, 20, 45, 3);

    filtered.copyTo(m_img);
    /*

    float resize = m_resize; // avoid lock
    cv::resize(m_img, src_color_sized, cv::Size(), resize, resize, cv::INTER_AREA);
//    cvtColor(src_color_sized, src_color_sized, COLOR_RGB2HLS_FULL);

    Mat hsv;
    cvtColor(src_color_sized, hsv, COLOR_BGR2HSV);

    // Quantize the hue to 30 levels
    // and the saturation to 32 levels
    int hbins = 100, sbins = 32;
    int histSize[] = {hbins, sbins};

    // hue varies from 0 to 179, see cvtColor
    float hranges[] = { 0, 180 };
    // saturation varies from 0 (black-gray-white) to
    // 255 (pure spectrum color)

    float sranges[] = { 0, 256 };
    const float* ranges[] = { hranges, sranges };

    MatND hist;
    // we compute the histogram from the 0-th and 1-st channels
    int channels[] = {0, 1};

    calcHist( &hsv,
              1,
              channels,
              Mat(),
              hist,
              2,
              histSize,
              ranges,
              true,
              false );



    double maxVal=0;
    minMaxLoc(hist, 0, &maxVal, 0, 0);
    int scale = 10;
    Mat histImg = Mat::zeros(sbins*scale, hbins*10, CV_8UC3);
    for( int h = 0; h < hbins; h++ )
        for( int s = 0; s < sbins; s++ )
        {
            float binVal = hist.at<float>(h, s);
            int intensity = cvRound(binVal*255/maxVal);
            rectangle( histImg,
                       Point(h*scale, s*scale),
                       Point( (h+1)*scale - 1, (s+1)*scale - 1),
                       Scalar::all(intensity),
                       -1 );
        }
  // cout << "test" << endl;
    histImg.copyTo(m_img);

    */

/*
    Mat labels, centers;
    int clusters = 5;
    double compactness = kmeans(src_color_sized,
                                clusters,
                                labels,
                                TermCriteria( TermCriteria::EPS+TermCriteria::COUNT, 10, 1.0),
                                1,
                                KMEANS_PP_CENTERS,
                                centers );

    cout << "kmeans" << endl;
*/
    return vector<UMat>();
}


void cvglCV::preprocessDenseFlow()
{

   // printf("hi \n");

    if( m_img.empty() )
    {
        return;
    }

    cv::resize(m_img, src_color_sized, cv::Size(), m_resize, m_resize, cv::INTER_AREA);
    cv::cvtColor(src_color_sized, src_gray, cv::COLOR_RGB2GRAY);


    if( m_invert )
    {
        // seems to have no effect on flow caclulation
        bitwise_not(src_gray, src_gray);
    }


    GaussianBlur(src_gray, src_blur_gray, cv::Size(m_gauss_ksize, m_gauss_ksize), m_gauss_sigma, m_gauss_sigma);
    erode( src_blur_gray, src_blur_gray, m_er_element );
    dilate( src_blur_gray, src_blur_gray, m_di_element );


    if( m_prev_frame.empty() ){
        m_prev_frame = src_blur_gray.clone();
        threshold_output = UMat();
        return;
    }

    dense_flow->calc(m_prev_frame, src_blur_gray, flow);

    m_prev_frame = src_blur_gray.clone();

    UMat mag_gray;

    // dist
    {
      //  double min, max; // debugging

        vector<UMat> flow_vec;
        split(flow, flow_vec);

        UMat dist, angle, angle_scaled;

        cartToPolar(flow_vec[0], flow_vec[1], dist, angle, true);

        // some scaling
        dist.convertTo(dist, CV_32F, 10);

        // used for threshold
        dist.convertTo(mag_gray, CV_8U, 1.);

        add(angle, 90.0f, angle); // 0-1

        Mat mask = angle.getMat(ACCESS_READ) > 180.0f;

        subtract(angle, 360.0f, angle, mask); // 0-1

        /*
        minMaxIdx(angle, &min, &max);
        cout << "angle " << min << " " << max << endl;
        cout << "angle type " << angle.type() << " dist type " <<  dist.type() << endl;
        */

        // manually scale to 0-255
       // multiply(dist, 255., dist);
       // multiply(angle_scaled, 255., angle_scaled);

/*
        minMaxIdx(dist, &min, &max);
        cout << "dist " << min << " " << max << endl;
        minMaxIdx(mag_gray, &min, &max);
        cout << "mag_gray " << min << " " << max << endl;
*/
        vector<UMat> xy_dist = { angle, dist, flow_vec[0], flow_vec[1] };

        UMat merged_xy_dist;
        merge(xy_dist, merged_xy_dist);

        merged_xy_dist.copyTo(src_color_sized);
        //merged_xy_dist.convertTo(src_color_sized, CV_8U);

/*

        vector<UMat> hsv_vec;
        hsv_vec.push_back(angle);
        hsv_vec.push_back(UMat::ones(angle.size(), angle.type()));
        hsv_vec.push_back(dist);

        UMat hsv;
        merge(hsv_vec, hsv);

        UMat img;
        cvtColor(hsv, img, COLOR_HSV2BGR);

        img.convertTo(img, CV_8UC3, 255);

        const double resize_scalar = 1.0 / m_resize;

        cv::resize(img, img, cv::Size(), resize_scalar, resize_scalar);
        img.copyTo(m_img);
*/

    }


    threshold( mag_gray, threshold_output, m_thresh, 255, cv::THRESH_BINARY );

    Sobel(src_gray, sob, CV_32F, 1, 1);
}




static void drawArrows(UMat& _frame, const vector<Point2f>& prevPts, const vector<Point2f>& nextPts, const vector<uchar>& status,
                       Scalar line_color = Scalar(0, 0, 255), double resize_scale = 1)
{
    Mat frame = _frame.getMat(ACCESS_WRITE);
    for (size_t i = 0; i < prevPts.size(); ++i)
    {
        if (status[i])
        {
            int line_thickness = 1;

            Point p = prevPts[i] * resize_scale;
            Point q = nextPts[i] * resize_scale;

            double angle = atan2((double) p.y - q.y, (double) p.x - q.x);

            double hypotenuse = sqrt( (double)(p.y - q.y)*(p.y - q.y) + (double)(p.x - q.x)*(p.x - q.x) );

            if (hypotenuse < 1.0)
                continue;

            // Here we lengthen the arrow by a factor of three.
            q.x = (int) (p.x - 3 * hypotenuse * cos(angle));
            q.y = (int) (p.y - 3 * hypotenuse * sin(angle));

            // Now we draw the main line of the arrow.
            line(frame, p, q, line_color, line_thickness);

            // Now draw the tips of the arrow. I do some scaling so that the
            // tips look proportional to the main line of the arrow.

            p.x = (int) (q.x + 9 * cos(angle + CV_PI / 4));
            p.y = (int) (q.y + 9 * sin(angle + CV_PI / 4));
            line(frame, p, q, line_color, line_thickness);

            p.x = (int) (q.x + 9 * cos(angle - CV_PI / 4));
            p.y = (int) (q.y + 9 * sin(angle - CV_PI / 4));
            line(frame, p, q, line_color, line_thickness);
        }
    }
}



void cvglCV::getFlow()
{
    
    //printf("hi \n");
    
    if( m_img.empty() )
    {
        return;
    }
    
    float resize = m_resize;
    cv::resize(m_img, src_color_sized, cv::Size(), resize, resize, cv::INTER_AREA);
    cv::cvtColor(src_color_sized, src_gray, cv::COLOR_RGB2GRAY);
    
    if( m_prev_frame.empty() ){
        m_prev_frame = src_gray.clone();
        return;
    }

    const double resize_scalar = 1.0 / resize;

    /*
    // pyrLK
    const size_t points = 100;

    vector<unsigned char> status(points);
    vector<float> err;

    m_track_points.clear();

    goodFeaturesToTrack(m_prev_frame, m_track_points, points, 0.01, 0.0);
    if(m_track_points.size() == 0)
        return;
    calcOpticalFlowPyrLK(m_prev_frame, src_gray, m_track_points, m_next_track_points, status, err);

    drawArrows(m_img, m_track_points, m_next_track_points, status, Scalar(255, 0, 0), resize_scalar);
*/

    dense_flow->calc(m_prev_frame, src_gray, flow);


    // display
    vector<UMat> flow_vec;
    split(flow, flow_vec);
    UMat magnitude, angle;

    /*
    multiply(flow_vec[0], flow_vec[0], sq_x );
    multiply(flow_vec[1], flow_vec[1], sq_y );
    add(sq_x, sq_y, sum);
    sqrt( sum, dist);
    dist.convertTo(dist, CV_32F, 0.01);
    */

    cartToPolar(flow_vec[0], flow_vec[1], magnitude, angle, true);
    magnitude.convertTo(magnitude, CV_32F, 0.01);

    vector<UMat> hsv_vec;
    hsv_vec.push_back(angle);
    hsv_vec.push_back(UMat::ones(angle.size(), angle.type()));
    hsv_vec.push_back(magnitude);

    UMat hsv;
    merge(hsv_vec, hsv);

    UMat img;
    cvtColor(hsv, img, COLOR_HSV2BGR);

    img.convertTo(img, CV_8UC3, 255);
    cv::resize(img, img, cv::Size(), resize_scalar, resize_scalar);



    //add(m_img, img, m_img);

    img.copyTo(m_img);


    m_prev_frame = src_gray.clone();


    /*

    pBackSub->apply(src_gray, fgMask);
    

    Mat mask(src_gray.size(), CV_8UC1);

    int maxCorners = 50;
    maxCorners = MAX(maxCorners, 1);
       vector<Point2f> corners;
       double qualityLevel = 0.05;
       double minDistance = 10;
       int blockSize = 3, gradientSize = 3;
       bool useHarrisDetector = false;
       double k = 0.04;
        if( m_prev_points.size() < 10 )
        {
            
            //Mat can;
            //cv::Canny(src_blur_gray, can, m_canny_min, m_canny_max, 3);
            goodFeaturesToTrack( fgMask,
                m_prev_points,
                maxCorners,
                qualityLevel,
                minDistance,
                mask,
                blockSize,
                gradientSize,
                useHarrisDetector,
                k );
        }
       
    
     //   cv::resize(mask, mask, cv::Size(), (1 / m_resize),  (1 / m_resize), cv::INTER_AREA);
    
       // add(m_img, mask, m_img);

  //  bitwise_not(m_img,m_img, mask);


        if( !m_prev_frame.empty() && m_prev_points.size() != 0 )
        {

            vector<uchar> status;
            vector<float> err;

            calcOpticalFlowPyrLK( m_prev_frame, fgMask, m_prev_points, corners, status, err);
            
            vector<Point2f> good_new;
            

            for(uint i = 0; i < m_prev_points.size(); i++)
            {
                // Select good points
                if(status[i] == 1) {
                    good_new.push_back(corners[i]);
                    // draw the tracks
                   // cout << corners[i] << endl;
                    line(m_img,
                         corners[i] * resize_scalar,
                         m_prev_points[i] * resize_scalar,
                         cv::Scalar(255,255,0), 1);
                    auto diff = (corners[i] - m_prev_points[i]);
                    float dist = cv::sqrt(diff.x*diff.x + diff.y*diff.y);

                    circle(m_img, corners[i] * resize_scalar, 1, cv::Scalar(255,255,0, 0.5) * dist, -1);
                }
            }
            
            m_prev_points = good_new;

        }
    else
    {
        m_prev_points = corners;
    }


    m_prev_frame = fgMask.clone();
    */
    
}

AnalysisData cvglCV::analyzeContour()
{
    AnalysisData data;
   // cout << "analyzeContour" << endl;

    if( threshold_output.empty() )
    {
        cout << "no image analyzeContour" << endl;
        return data;
    }
    
   // Mat thesh = threshold_output.getMat(ACCESS_READ);
    findContours( threshold_output, data.contours, data.hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );
    
    size_t npix = threshold_output.rows * threshold_output.cols;
    
    data.halfW = threshold_output.cols / 2.0f;
    data.halfH = threshold_output.rows / 2.0f;

    vector< double > contour_area;

    for( size_t i = 0; i < data.contours.size(); i++ )
    {
        double contour_a = contourArea( data.contours[i] ) / npix;
        
        if( (contour_a < m_minsize) || (contour_a > m_maxsize ) || ((m_parents_only && (data.hierarchy[i][3] != -1))) )
            continue;
        
        contour_area.emplace_back(contour_a);
        data.contour_idx.emplace_back(i);
        
        // hull
        Mat hullP, hullI;
        cv::RotatedRect minRect;
        vector<Vec4i> defects;
        
        cvgl::minAreaRectHull( data.contours[i], minRect, hullP, hullI );
        
        
        size_t hullI_size = hullI.rows * hullI.cols;
        if( hullI_size > 3 )
            cvgl::convexityDefects( data.contours[i], hullI, defects );
        
        data.hullP_vec.emplace_back( hullP );
        data.hullI_vec.emplace_back( hullI );
        data.defects_vec.emplace_back( defects );
        data.minRect_vec.emplace_back(minRect);
        
    }
    
    
    data.initSizeFromIdx();
    data.contour_area = Eigen::Map<Eigen::ArrayXd, Eigen::Aligned>(contour_area.data(), contour_area.size());
    
   // lock_guard<mutex> lock(m_lock);
   
    m_thread_pool->enqueue([this](AnalysisData _data){ analysisThread(_data);}, data);
    
   // thread worker(&cvglCV::analysisThread, this, data);
    //worker.detach();
    
    return data;// std::move(data);
}


void cvglCV::analysisThread(AnalysisData data)
{
    unique_lock<mutex> lock(m_lock);

        const AnalysisData prev_data = m_prev_data;
        vector<int> id_used = m_id_used;

        Mat src_color_sized_mat, sob_mat;

        src_color_sized.copyTo(src_color_sized_mat);
        sob.copyTo(sob_mat);

    m_lock.unlock();
    
    int nchans = src_color_sized_mat.channels();
    double src_width = (double)src_color_sized_mat.size().width;
    double src_height = (double)src_color_sized_mat.size().height;
    double npix = src_width * src_height;

    data.centroids.reserve( data.ncontours );
    
    const bool float_mat = src_color_sized_mat.depth() == CV_32F || src_color_sized_mat.depth() == CV_64F;

    for( size_t i = 0; i < data.ncontours; i++ )
    {
        const Mat& contour = data.contours[ data.contour_idx[i] ];
        
        
        Mat contour_mask = Mat::zeros( src_color_sized_mat.size(), CV_8UC1 );
        drawContours(contour_mask, data.contours, data.contour_idx[i], Scalar(255), FILLED);
        
        cv::Rect boundRect = boundingRect( contour );
        

        vector<PixStats> stats;
        if( float_mat )
            stats = getStatsFloat( src_color_sized_mat, sob_mat, contour_mask, boundRect );
        else
            stats = getStatsChar( src_color_sized_mat, sob_mat, contour_mask, boundRect );

        if (m_use_preprocess != 3 && m_color_mode == 2 && stats.size() == 3) {
            stats[0].mean *= 0.3921568627; // 100/255 to sacle to 0-100
            stats[1].mean -= 128;
            stats[2].mean -= 128;
// scale std? add std? to stats?
            
            /* will have to deal with this from the plastisphere scripts
             
             if( bound(/pix/l/mean),
               progn(
                 /pix/l/mean *= 0.3921568627, # 100/255
                 /pix/a/mean -= 128,
                 /pix/b/mean -= 128,
                 /pix/l/std = sqrt(/pix/l/var) * 0.3921568627,
                 /pix/a/std = sqrt(/pix/a/var),
                 /pix/b/std = sqrt(/pix/b/var),

                 /pix/l/max = /pix/l/mean + /pix/l/std,
                 /pix/a/max = /pix/a/mean + /pix/a/std,
                 /pix/b/max = /pix/b/mean + /pix/b/std

               )
             )
             */
        }
        
        data.pix_channel_stats[i] = stats;

        data.focus(i) = stats[ src_color_sized_mat.channels() ].variance;
        
        data.parent(i) = data.hierarchy[ data.contour_idx[i] ][3] ;
        data.parimeter(i) = arcLength(contour, true) ;
        
        cv::RotatedRect& minRect = data.minRect_vec[i];
        
        double hh = minRect.size.height / src_height;
        double ww = minRect.size.width / src_width;
        
        double major = max(hh, ww);
        double minor = min(hh, ww);
        
        data.rotrect_minor(i) = minor;
        data.rotrect_major(i) = major;
        
        double centerx = minRect.center.x;
        double centery = minRect.center.y;
        
        data.center_x(i) =  centerx / src_width ;
        data.center_y(i) =  1. - (centery / src_height) ;
        
        double _a = major / 2.;
        double _b = minor / 2.;
        double ecc = sqrt(1 - pow(_b/_a, 2));
        
        data.eccentricity(i) =  ecc ;
        
        
        double r_angle = minRect.angle;
        double out_angle = 0.0;
        if( minRect.size.height > minRect.size.width )
        {
            out_angle = -r_angle + 90.0;
        }
        else
        {
            out_angle = -r_angle;
        }
        
        data.angle(i) = out_angle ;
        
        
        data.size_x(i) =  boundRect.width / src_width ;
        data.size_y(i) =  boundRect.height / src_height ;
        
        double ctrdx = -1;
        double ctrdy = -1;
        
        Moments moms = moments( contour );
        
        // Hu momemnts
        double hu[7];
        HuMoments(moms, hu);
        
        //  obj.contourPts.addMessage( "/hu", vector<double>(hu, hu+7) );
        
        if( moms.m00 != 0.0 )
        {
            ctrdx = moms.m10 / moms.m00;
            ctrdy = moms.m01 / moms.m00;
            
        }
        
        double scaled_centroidX = ctrdx / src_width;
        double scaled_centroidY = 1. - (ctrdy / src_height);
        
        data.centroids.emplace_back(cv::Point2f(scaled_centroidX, scaled_centroidY));
        
        data.centroid_x(i) =  scaled_centroidX ;
        data.centroid_y(i) =  scaled_centroidY ;
        
        // centroids.push_back( Point2f(ctrdx, ctrdy) );
        
        data.convex(i) = cvgl::isContourConvex( contour ) ;
        
        
        Mat convexcontour;
        approxPolyDP( data.hullP_vec[i], convexcontour, 0.001, true);
        
        data.hull_area(i) =  contourArea(convexcontour) / npix ;
        
        data.defect_count(i) =  (int)data.defects_vec[i].size() ;
        
        data.hull_count(i) =  (int)data.hullI_vec[i].rows * (int)data.hullI_vec[i].cols ;
        
        
        double graph_centroid_x = cvgl::scale( data.centroid_x(i), 0., 1., -1., 1.);
        double graph_centroid_y = cvgl::scale( data.centroid_y(i), 0., 1., -1., 1.);

        data.centroid_dist(i) = sqrt( graph_centroid_x*graph_centroid_x + graph_centroid_y*graph_centroid_y );
        data.centroid_angle(i) = atan2(graph_centroid_y, graph_centroid_y ) * 57.2957795131;
        
        double aspect = src_width / src_height;
        
        data.elongation(i) = (data.size_x(i) * aspect) / data.size_y(i);
        data.verticality(i) = sin( data.angle(i) * 0.01745329252);
        
        double dist_sum = 0;
//        vector<Vec4i>::iterator d =  data.defects_vec[i].begin();
//        vector<Vec4i>::iterator d_end =  data.defects_vec[i].end();
        
        ArrayXd defect_x(data.defects_vec[i].size());
        ArrayXd defect_y(data.defects_vec[i].size());
        ArrayXd defect_depth(data.defects_vec[i].size());
        
       // cout << "rows " << data.contours[i].rows << " columns " <<  data.contours[i].cols << " depth " << (data.contours[i].depth() == CV_32S) << endl;
        
        for( size_t j = 0; j < data.defects_vec[i].size(); ++j )
        {
            Vec4i& v = data.defects_vec[i][j];
            cv::Point2i * ptFar = data.contours[i].ptr<cv::Point2i>( v[2] );
            defect_x(j) = (double)ptFar->x / src_width;
            defect_y(j) = (double)ptFar->y / src_height;
            defect_depth(j) = (double)v[3] / 256.;
            
            dist_sum += defect_depth(j);
        }
        
        ArrayXd rel_x = defect_x - data.centroid_x(i);
        ArrayXd rel_y = defect_y - data.centroid_y(i);
        ArrayXd defect_theta =  rel_y.binaryExpr(rel_x, [](double a, double b) { return std::atan2(a,b);} );
        ArrayXd defect_depthweight = defect_depth / dist_sum;
        
        ArrayXd defect_w_x = ( defect_depthweight * defect_theta.sin() );
        ArrayXd defect_w_y = ( defect_depthweight * defect_theta.cos() );

        data.defect_rel_mean_angle = std::atan2( defect_w_x.sum(),  defect_w_y.sum() ) * 57.2957795131;

        data.defect_dist_sum(i) = dist_sum;
        
    }
    
            
    // id tracking
    
    if( prev_data.centroids.size() == 0 )
    {
        
        for( size_t i = 0; i < data.centroids.size(); i++ )
        {
            id_used[i] = 1;
            data.id.emplace_back(i);
            data.id_idx.emplace(i, i);
            
            
        }
        
    }
    else
    {
        // if prev centroids are accounted for, then keep the same ids, if not found release id for prev centroid
        data.id = vector<int>( data.centroids.size(), -1 );
        
        int closest_idx = -1;
        double radius_max = m_track_radius * src_height;
        double min = radius_max;
        int debug_count = 0;
        
        data.sustain_idx.reserve( data.centroids.size() );
        data.noteOn_idx.reserve( data.centroids.size() );
        data.noteOff_prev_idx.reserve( prev_data.centroids.size() );

        // get delta movement between tracked contours
        
        
        // fist check if previous points are found
        for( size_t j = 0; j < prev_data.centroids.size(); j++ )
        {
            
            min = radius_max;
            closest_idx = -1;
            debug_count = 0;
            
            for( size_t i = 0; i < data.centroids.size(); i++ )
            {
                
                double delta = norm(data.centroids[i] - prev_data.centroids[j]);
                
                // if within range and if not yet assigned, do assignment
                if( delta <= radius_max && data.id[i] == -1 )
                {
                    if( min >= delta )
                    {
                        min = delta;
                        closest_idx = i;
                        // could maybe break here once we find something within the range,
                        // but currnetly we look through *every* possibility and find the closest
                    }
                }
                
            }
            
            if( closest_idx > -1 )
            {
                // selected a closest match index
                data.id[closest_idx] = prev_data.id[j];
                data.id_idx.emplace( prev_data.id[j], closest_idx );
                data.sustain_idx.emplace_back( closest_idx );
            }
            else
            {
                // not found, so fee the id slot
                id_used[ prev_data.id[j] ] = 0;
                data.noteOff_prev_idx.emplace_back(j);
            }
            
        }
        
        // check for unassigned new_ids, and then find the first unused id number:
        for( size_t i = 0; i < data.centroids.size(); i++ )
        {
            if( data.id[i] == -1 )
            {
                for( size_t n = 0; n < m_maxIDs; n++ )
                {
                    if( id_used[n] == 0)
                    {
                        data.id[i] = n;
                        data.id_idx.emplace(n, i);
                        data.noteOn_idx.emplace_back(i);

                        id_used[n] = 1;
                        break;
                    }
                }
            }
            
        }
        

    }
    
    
    
    
    /*
    for( int i = 0; i < data.id.size(); i++)
    {
        cout << data.id[i] << " " << i << " " << data.id_idx[ data.id[i] ] << endl;
    }*/
    
    // >> should now be possible to do deltas between data, using id_idx to look up index based on tracked ids
    

    analysisTracking(data, prev_data);
    
    processAnalysis(data);
    
    m_lock.lock();
    m_prev_data = data;
    m_id_used = id_used;
    m_lock.unlock();
    
}

//typedef std::chrono::milliseconds ms_t;

void cvglCV::analysisTracking(AnalysisData& data, const AnalysisData& prev_data)
{
    
    auto time_now = std::chrono::system_clock::now();
    
    // note: preset delta varaibles to zero in init function in case there are no matching ids from prev frame
    for( size_t i = 0; i < data.ncontours; ++i )
    {
        auto prev_ref = prev_data.id_idx.find( data.id[i] );
        if( prev_ref != prev_data.id_idx.end() && prev_data.ncontours > 0 )
        {

            // found id in previous frame, can be used to get delta
            int prev_idx = prev_ref->second;

            double dx = data.centroid_x(i) - prev_data.centroid_x(prev_idx);
            double dy = data.centroid_y(i) - prev_data.centroid_y(prev_idx);

            data.delta_centroid_x(i) = dx;
            data.delta_centroid_y(i) = dy;
            data.delta_centroid_dist(i) = sqrt( dx*dx + dy*dy );                        
            data.start_centroid_x(i) = prev_data.start_centroid_x(prev_idx);
            data.start_centroid_y(i) = prev_data.start_centroid_y(prev_idx);            
            data.start_time[i] = prev_data.start_time[prev_idx];
            std::chrono::duration<float> dur = time_now - data.start_time[i];
            data.elapsed_contour(i) = dur.count();


        }
        else
        {
            data.start_centroid_x(i) = data.centroid_x(i);
            data.start_centroid_y(i) = data.centroid_y(i);
            data.delta_centroid_dist(i) = 0;
            data.delta_centroid_x(i) = 0;
            data.delta_centroid_y(i) = 0;
            data.start_time[i] = time_now;
            data.elapsed_contour(i) = 0;
        }
        
    }
    
}


vector<PixStats> cvglCV::getStatsChar( const Mat& src, const Mat& sobel, const Mat& mask, const cv::Rect& roi)
{
    //const int plane, T& min, T& max, T& varience
    
    // test roi
    if( src.size() != mask.size() )
    {
        printf("size mismatch\n");
        return vector<PixStats>();
    }
    
    int nchans = src.channels();
    int nstats = nchans + 1; // focus (removed flow)
    
    int focus = nchans;
    
    vector<PixStats> stats( nstats );
    
    vector<cv::Point> index;
    index.reserve( roi.area() );
    
    const uchar *mask_p = NULL;
    const uchar *src_p = NULL;
    const float *sobel_p = NULL;
    
    int row_start = roi.y;
    int row_end = roi.y + roi.height;
    
    int col_start = roi.x;
    int col_end = col_start + roi.width;
    
    //const cv::Mat src_mat = src.getMat(ACCESS_READ);
    //const cv::Mat sob_mat = sobel.getMat(ACCESS_READ);

    for( int i = row_start; i < row_end; ++i )
    {
        mask_p = mask.ptr<uchar>(i);
        
        // do type check above here, eventually would be nice to support float also
        src_p = src.ptr<uchar>(i);
        sobel_p = sobel.ptr<float>(i);
        
        for( int j = col_start; j < col_end; ++j )
        {
            if( mask_p[j] )
            {
                index.push_back( cv::Point(j, i) );
                
                
                // src
                for( int c = 0; c < nchans; ++c)
                {
                    const uchar val = src_p[ (j*nchans) + c];
                    
                    if( val < stats[c].min )
                        stats[c].min = val;
                    
                    if( val > stats[c].max )
                        stats[c].max = val;
                    
                    
                    stats[c].sum += (double)val;
                    
                }
                
                //sobel
                if( sobel_p[j] < stats[focus].min )
                    stats[focus].min = sobel_p[j];
                
                if( sobel_p[j] > stats[focus].max )
                    stats[focus].max = sobel_p[j];
                
                stats[focus].sum += (double)sobel_p[j];
                
            }
            
        }
    }
    
    size_t size = index.size();
    if( size == 0 )
        ;//cout << "size zero" << endl;
    
    for( int c = 0; c < nchans; ++c)
    {
        stats[c].mean = stats[c].sum / size;
    }

    stats[focus].mean = stats[focus].sum / size;

    int row, col;
    for( size_t i = 0; i < size; ++i )
    {
        col = index[i].x;
        row = index[i].y;
        
        src_p = src.ptr<uchar>(row);
        sobel_p = sobel.ptr<float>(row);
        
        for( int c = 0; c < nchans; ++c)
        {
            double dx = src_p[ (col*nchans) + c ] - stats[c].mean;
            stats[c].dev_sum += (dx*dx);
        }
        
        double dx = sobel_p[ col ] - stats[focus].mean;
        stats[focus].dev_sum += (dx*dx);
        
    }
    
    double v_size = size - 1;
    
    for( int c = 0; c < nchans; ++c)
    {
        stats[c].variance = stats[c].dev_sum / v_size;
    }
    
    stats[focus].variance = stats[focus].dev_sum / v_size;
    

    return stats;
    
}


vector<PixStats> cvglCV::getStatsFloat( const Mat& src, const Mat& sobel, const Mat& mask, const cv::Rect& roi)
{
    //const int plane, T& min, T& max, T& varience

   // cout << "getStatsFloat" << endl;

    // test roi
    if( src.size() != mask.size() )
    {
        printf("size mismatch\n");
        return vector<PixStats>();
    }

    int nchans = src.channels();
    int nstats = nchans + 1; // focus (removed flow)

    int focus = nchans;

    vector<PixStats> stats( nstats );

    vector<cv::Point> index;
    index.reserve( roi.area() );

    const float *src_p = NULL;
    const uchar *mask_p = NULL;
    const float *sobel_p = NULL;

    int row_start = roi.y;
    int row_end = roi.y + roi.height;

    int col_start = roi.x;
    int col_end = col_start + roi.width;

    //const cv::Mat src_mat = src.getMat(ACCESS_READ);
    //const cv::Mat sob_mat = sobel.getMat(ACCESS_READ);

    for( int i = row_start; i < row_end; ++i )
    {
        mask_p = mask.ptr<uchar>(i);

        // do type check above here, eventually would be nice to support float also
        src_p = src.ptr<float>(i);
        sobel_p = sobel.ptr<float>(i);

        for( int j = col_start; j < col_end; ++j )
        {
            if( mask_p[j] )
            {
                index.push_back( cv::Point(j, i) );


                // src
                for( int c = 0; c < nchans; ++c)
                {
                    const float val = src_p[ (j*nchans) + c];

                    if( val < stats[c].min )
                        stats[c].min = val;

                    if( val > stats[c].max )
                        stats[c].max = val;


                    stats[c].sum += val;

                }

                //sobel
                if( sobel_p[j] < stats[focus].min )
                    stats[focus].min = sobel_p[j];

                if( sobel_p[j] > stats[focus].max )
                    stats[focus].max = sobel_p[j];

                stats[focus].sum += sobel_p[j];

            }

        }
    }

    size_t size = index.size();
    if( size == 0 )
        ;// cout << "size zero" << endl;

    for( int c = 0; c < nchans; ++c)
    {
        stats[c].mean = stats[c].sum / size;
    }

    stats[focus].mean = stats[focus].sum / size;

    int row, col;
    for( size_t i = 0; i < size; ++i )
    {
        col = index[i].x;
        row = index[i].y;

        src_p = src.ptr<float>(row);
        sobel_p = sobel.ptr<float>(row);

        for( int c = 0; c < nchans; ++c)
        {
            double dx = src_p[ (col*nchans) + c ] - stats[c].mean;
            stats[c].dev_sum += (dx*dx);
        }

        double dx = sobel_p[ col ] - stats[focus].mean;
        stats[focus].dev_sum += (dx*dx);

    }

    double v_size = size - 1;

    for( int c = 0; c < nchans; ++c)
    {
        stats[c].variance = stats[c].dev_sum / v_size;
    }

    stats[focus].variance = stats[focus].dev_sum / v_size;

    return stats;

}


