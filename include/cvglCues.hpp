#pragma once

#include <chrono>
#include <iostream>

#include <Eigen/Dense>
#include "cvglAnalysisData.hpp"
#include "MapOSC.hpp"
//#include "cvglMixer.hpp"
#include "cvglRandom.hpp"
#include "cvglLookupFigure.hpp"

#ifndef CUE_FILE_NAMES
#define CUE_FILE_NAMES "test"
#endif

/**
 * in the future, probably it will be easier if the cues are static functions,
 * and recieve all the of state and reference pointers that they need from the caller
 * that way it will be easier to pass function pointers, passing method pointers is tricky
 *

 something like:

    typedef struct cueArgs {
        MapOSC& cache;
        const AnalysisData& data;
        MapOSC& b;
        cvglRandom& randGen;
        timepoint_t startTime;
        duration_t elapsed_section;
     } cueArgs_t;

     and then pass is struct as the arg to the cues, which passes references to the class state cache etc.

 */

typedef std::chrono::system_clock sys_clock_t;
typedef std::chrono::time_point<sys_clock_t> timepoint_t;
typedef std::chrono::duration<double> duration_t;

struct cueArgs {
    MapOSC& cache;
    const AnalysisData& data;
    MapOSC& b;
    cvglRandom& randGen;
    timepoint_t startTime;
    duration_t elapsed_section;
    bool isNewCue = false;
};


class cvglCues
{
public:

    typedef std::function< MapOSC(cueArgs) > cueFunction_t;

    struct LabeledCue {
        cueFunction_t fn;
        string descr;
        string next_cue;
    };


    /**
     processes data and input settings, outputs OSC bundle to send to audio process
     */
    MapOSC procDataAndMixer(const AnalysisData& data, MapOSC& b);
    MapOSC procDataAndMixer(const AnalysisData& data);

    bool isNewCue = true;

    vector<string> getCueNames()
    {
        vector<string> cuenames;
        for( auto& c : m_cueFunctions )
        {
            cuenames.emplace_back( c.first );
        }

        sort(cuenames.begin(), cuenames.end());
        return cuenames;
    }

    void setCue(string label, string descr, string next_cue, cueFunction_t fn)
    {
        m_cueFunctions.emplace(label, LabeledCue({ fn, descr, next_cue}) );
    }


protected:



    MapOSC m_state_cache;
    
    string m_cue = "0";

    timepoint_t m_section_start = sys_clock_t::now();
    duration_t m_elapsed_section;
    
    unordered_map<string, LabeledCue> m_cueFunctions;
    
    cvglRandom m_rand_generator;

    MapOSC m_input;
    
    AnalysisData m_prev_data;
};


//   void set_lambda_cues();


//   MapOSC cue0(const AnalysisData& data, MapOSC& b);
//   MapOSC cue1(const AnalysisData& data, MapOSC& b);


/*
 *
    cvglCues()
    {
        // not doing this right now but maybe at somepoint, could be a way to dynamically load
        // all the cues without having to manually type them in the header file.


     //   cout << "****** CUE_FILE_NAMES ***** " << CUE_FILE_NAMES << endl;


     //   using namespace std::placeholders;
     //   m_cueFunctions.emplace_back( bind(&cvglCues::cue0, this, _1, _2) );
     //   m_cueFunctions.emplace_back( bind(&cvglCues::cue1, this, _1, _2) );
  //      set_lambda_cues();


    }
*/

/* this is probably the better way to do this, but I'm having trouble getting the function to bind
typedef struct cvglCue {
    cueFunction_t fn;
    string descr;
    string next_cue;
    cvglCue(cueFunction_t fn_, string descr_, string nextdescr_) : fn(fn_), descr(descr_), next_cue(nextdescr_) {}
} cvglCue_t;
*/
