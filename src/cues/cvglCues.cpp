
#include "cvglCues.hpp"

// called from analysis thread
MapOSC cvglCues::procDataAndMixer(const AnalysisData& data)
{
    
    m_elapsed_section = sys_clock_t::now() - m_section_start;
    
    MapOSC out = m_cueFunctions[m_cue].fn(
            cueArgs({ m_state_cache,
                      data,
                      m_input,
                      m_rand_generator,
                      m_section_start,
                      m_elapsed_section
                    }));
    
    isNewCue = false;

    return out;
}

// called from UDP input thread
MapOSC cvglCues::procDataAndMixer(const AnalysisData& data, MapOSC& b)
{
    MapOSC out ;
    bool set = false;
    if( b.addressExists("/cue") )
    {
        string cue = b["/cue"].get<string>();
        if( cue != m_cue && m_cueFunctions.count(cue) > 0 )
        {
            cout << "setting new cue " << cue << " was " << m_cue << endl;

            m_cue = cue;
            isNewCue = true;

            m_section_start = sys_clock_t::now();

            m_state_cache.addMessage("/played", 0);

            m_elapsed_section = sys_clock_t::now() - m_section_start;

            out = m_cueFunctions[m_cue].fn(
                    cueArgs({   m_state_cache,
                                data,
                                b,
                                m_rand_generator,
                                m_section_start,
                                m_elapsed_section,
                                isNewCue
                            }));
           // out = m_cueFunctions[m_cue]( data, b );

            out.addMessage("/descr", m_cueFunctions[m_cue].descr);
            out.addMessage("/next_cue", m_cueFunctions[m_cue].next_cue);
        }
        
    }
    else if( b.addressExists("/set") )
    {
        set = true;
    }
    else
    {
        m_elapsed_section = sys_clock_t::now() - m_section_start;
        //out = m_cueFunctions[m_cue]( data, b );
        out = m_cueFunctions[m_cue].fn(
                cueArgs({     m_state_cache,
                              data,
                              b,
                              m_rand_generator,
                              m_section_start,
                              m_elapsed_section
                          }));
    }
    

    // this shoud copy bundle
    m_input = MapOSC(b); // acutally unused now
    isNewCue = false;
    
    return !set ? out : MapOSC();
}
