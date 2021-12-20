#pragma once

#include "opencv2/core/utility.hpp"
#include <Eigen/Dense>
#include <unordered_map>

struct PixStats {
    double min = std::numeric_limits<double>::max();
    double max = 0;
    double mean = 0;
    double sum = 0;
    double dev_sum = 0;
    double variance = 0;
};

// convert everything to eigen here?

struct AnalysisData
{
    
    double                              halfW, halfH;
    
    size_t                              ncontours = 0;
    std::vector< cv::Mat >              contours;
    
    std::vector< int >                  contour_idx;
    std::vector< cv::Vec4i >            hierarchy;
    std::vector< cv::Mat >              hullP_vec;
    std::vector< cv::Mat >              hullI_vec;
    
    std::vector< std::vector< cv::Vec4i > > defects_vec;
    std::vector< cv::RotatedRect >          minRect_vec;
    
    // vector 1: contour id
    // vector 2: stats for each input channel
            // note: a focus channel is added for the focus mean for each contour
    std::vector< std::vector< PixStats > >  pix_channel_stats;
    
    std::vector<cv::Point2f>            centroids;
    
    // data used for mapping:
   // Eigen::ArrayXi          id;
    Eigen::ArrayXi          parent; // (idx)
    
    Eigen::ArrayXd          contour_area;
    Eigen::ArrayXd          hull_area;
    Eigen::ArrayXi          hull_count;

    Eigen::ArrayXd          center_x;
    Eigen::ArrayXd          center_y;
    Eigen::ArrayXd          centroid_x;
    Eigen::ArrayXd          centroid_y;
    

    
    Eigen::ArrayXd          size_x;
    Eigen::ArrayXd          size_y;
    Eigen::ArrayXd          parimeter;
    Eigen::ArrayXd          angle;
    Eigen::ArrayXd          eccentricity;
    Eigen::ArrayXd          rotrect_minor;
    Eigen::ArrayXd          rotrect_major;
    Eigen::ArrayXd          focus;
    
    Eigen::ArrayXd          defect_dist_sum;
    Eigen::ArrayXi          defect_count;
    Eigen::ArrayXi          convex;
    
    Eigen::ArrayXd          elongation;
    Eigen::ArrayXd          verticality;
    Eigen::ArrayXd          centroid_dist; // from center
    Eigen::ArrayXd          centroid_angle;// from center
    
    // maybe useful someday, but leaving out for now
    //Eigen::ArrayXd          defect_rel_x; // rel from centroid
    //Eigen::ArrayXd          defect_rel_y;
    //Eigen::ArrayXd          defect_rel_angle;
    //Eigen::ArrayXd          defect_rel_depthweight;
    Eigen::ArrayXd          defect_rel_mean_angle;
    
    
    Eigen::ArrayXd          elapsed_contour;
    Eigen::ArrayXd          start_centroid_x;
    Eigen::ArrayXd          start_centroid_y;
    
    Eigen::ArrayXd          delta_centroid_x;
    Eigen::ArrayXd          delta_centroid_y;
    Eigen::ArrayXd          delta_centroid_dist;
    
    // idx refers to list order in arrays
    std::vector<int32_t>    noteOn_idx;
    std::vector<int32_t>    sustain_idx;
    std::vector<int32_t>    noteOff_prev_idx;

    std::vector<int> id; // idx -> id lookup
    std::unordered_map<int, int> id_idx;  // id -> idx lookup
    
    std::vector< std::chrono::time_point<std::chrono::system_clock> > start_time;
    
    void initSizeFromIdx()
    {
        size_t n = contour_idx.size();
        ncontours = n;
        
       // id.reserve(n); // set to -1 in analysis function
        id_idx.reserve( n );
        
        start_time.resize( n );
        
        parent.resize(n);
        contour_area.resize(n);
        hull_area.resize(n);
        hull_count.resize(n);
        center_x.resize(n);
        center_y.resize(n);
        centroid_x.resize(n);
        centroid_y.resize(n);

        size_x.resize(n);
        size_y.resize(n);
        parimeter.resize(n);
        angle.resize(n);
        eccentricity.resize(n);
        centroid_y.resize(n);
        rotrect_minor.resize(n);
        rotrect_major.resize(n);
        focus.resize(n);
        defect_dist_sum.resize(n);
        defect_count.resize(n);
        convex.resize(n);
        
        elongation.resize(n);
        verticality.resize(n);
        centroid_dist.resize(n);
        centroid_angle.resize(n);
        /*
        defect_rel_x.resize(n);
        defect_rel_y.resize(n);
        defect_rel_angle.resize(n);
        defect_rel_depthweight.resize(n);
         */
        defect_rel_mean_angle.resize(n);
        
        delta_centroid_x = Eigen::ArrayXd::Zero(n);
        delta_centroid_y = Eigen::ArrayXd::Zero(n);
        delta_centroid_dist = Eigen::ArrayXd::Zero(n);
        
        start_centroid_x.resize(n);
        start_centroid_y.resize(n);
        elapsed_contour = Eigen::ArrayXd::Zero(n);
        
        // add pixel data here
        pix_channel_stats.resize(n);
    }
};

