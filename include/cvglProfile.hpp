#pragma once

#include <chrono>
#include <iostream>


class cvglProfile
{
public:
    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
    
    void markStart()
    {
        start = std::chrono::system_clock::now();
    }
    
    void markEnd(std::string id = "")
    {
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end-start;
        std::time_t end_time = std::chrono::system_clock::to_time_t(end);
        //std::cout << "finished computation at " << std::ctime(&end_time);
      //  double dur = elapsed_seconds.count();
     //   if( dur > max ) max = dur;
        std::cout << id << " elapsed time: \t" << elapsed_seconds.count() <<  "s " /* << max << ")" */ << std::endl;
    }
    
    double max = 0;
};
