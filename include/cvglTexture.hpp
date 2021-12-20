#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "opencv2/core/utility.hpp"

class cvglTexture
{
public:
    cvglTexture()
    {
        glGenTextures(1, &m_tex);
        glBindTexture(GL_TEXTURE_2D, m_tex);
        
        // wrap parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // interpolation
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // Mipmap not sure if this is supposed to be used with the interpolation or not
        glGenerateMipmap(GL_TEXTURE_2D);
        
        
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    ~cvglTexture()
    {
        glDeleteTextures(1, &m_tex);
        m_tex = 0;
    }
    
    void bind()
    {
    //    glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_tex);
    }
    
    void setTexture(cv::Mat bgr_mat)
    {
        glBindTexture(GL_TEXTURE_2D, m_tex);
        
        glTexImage2D(GL_TEXTURE_2D,     // Type of texture
                     0,                 // Pyramid level (for mip-mapping) - 0 is the top level
                     GL_RGB,            // Internal colour format to convert to
                     bgr_mat.cols,          // Image width  i.e. 640 for Kinect in standard mode
                     bgr_mat.rows,          // Image height i.e. 480 for Kinect in standard mode
                     0,                 // Border width in pixels (can either be 1 or 0)
                     GL_BGR, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
                     GL_UNSIGNED_BYTE,  // Image data type
                     bgr_mat.ptr());        // The actual image data itself
    }
    
    void setTextureBW(cv::Mat bw_mat)
    {
        glBindTexture(GL_TEXTURE_2D, m_tex);
        glTexImage2D(GL_TEXTURE_2D,     // Type of texture
                     0,                 // Pyramid level (for mip-mapping) - 0 is the top level
                     GL_RGB,            // Internal colour format to convert to
                     bw_mat.cols,          // Image width  i.e. 640 for Kinect in standard mode
                     bw_mat.rows,          // Image height i.e. 480 for Kinect in standard mode
                     0,                 // Border width in pixels (can either be 1 or 0)
                     GL_RED, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
                     GL_UNSIGNED_BYTE,  // Image data type
                     bw_mat.ptr());        // The actual image data itself
    }
    
    void setTexture( float rgba[4] )
    {
        glBindTexture(GL_TEXTURE_2D, m_tex);
        glTexImage2D(GL_TEXTURE_2D,     // Type of texture
                     0,                 // Pyramid level (for mip-mapping) - 0 is the top level
                     GL_RGBA,            // Internal colour format to convert to
                     1,          // Image width  i.e. 640 for Kinect in standard mode
                     1,          // Image height i.e. 480 for Kinect in standard mode
                     0,                 // Border width in pixels (can either be 1 or 0)
                     GL_RGBA, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
                     GL_FLOAT,  // Image data type
                     rgba);        // The actual image data itself
    }
    
    void setTexture( std::vector<float> rgba )
    {
        glBindTexture(GL_TEXTURE_2D, m_tex);
        glTexImage2D(GL_TEXTURE_2D,     // Type of texture
                     0,                 // Pyramid level (for mip-mapping) - 0 is the top level
                     GL_RGBA,            // Internal colour format to convert to
                     1,          // Image width  i.e. 640 for Kinect in standard mode
                     1,          // Image height i.e. 480 for Kinect in standard mode
                     0,                 // Border width in pixels (can either be 1 or 0)
                     GL_RGBA, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
                     GL_FLOAT,  // Image data type
                     rgba.data());        // The actual image data itself
    }
    
    void setTexture( float r, float g, float b, float a )
    {
        float rgba[4] = {r,g,b,a};
        glBindTexture(GL_TEXTURE_2D, m_tex);
        glTexImage2D(GL_TEXTURE_2D,     // Type of texture
                     0,                 // Pyramid level (for mip-mapping) - 0 is the top level
                     GL_RGBA,            // Internal colour format to convert to
                     1,          // Image width  i.e. 640 for Kinect in standard mode
                     1,          // Image height i.e. 480 for Kinect in standard mode
                     0,                 // Border width in pixels (can either be 1 or 0)
                     GL_RGBA, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
                     GL_FLOAT,  // Image data type
                     rgba);        // The actual image data itself
    }
    
    inline GLuint getTexID(){ return m_tex; }
    
private:
    GLuint m_tex = 0;
    
};
