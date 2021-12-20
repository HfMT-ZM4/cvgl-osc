#pragma once

#include "cvglCV.hpp"

struct cvglGaussianBlur
{
    
    void operator() (cv::InputArray src, cv::OutputArray dst) {
        cv::GaussianBlur(src, dst, cv::Size(m_gauss_ksize, m_gauss_ksize), m_gauss_sigma, m_gauss_sigma);
    }
    

    
    int m_gauss_sigma = 3;
    int m_gauss_ksize = m_gauss_sigma*5;
    cv::Mat m_er_element = cv::getStructuringElement( cv::MORPH_RECT, cv::Size(1,1), cv::Point(0,0) );
    cv::Mat m_di_element = getStructuringElement( cv::MORPH_RECT, cv::Size(1,1), cv::Point(0,0) );
    
};
