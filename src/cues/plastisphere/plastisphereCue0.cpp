#include "cvglCues.hpp"

MapOSC cvglCues::cue0( const AnalysisData& data, MapOSC& b )
{
    //b.print();
    MapOSC out;
    out.addMessage("/descr", "prelude: setting up, putting on gloves");
    
    const double elapsed_section = m_elapsed_section.count();
    
    
    if( isNewCue )
    {
        out.addMessage("/pregain/perc",      -6);
        out.addMessage("/pregain/gran",      -70);
        out.addMessage("/pregain/res",       -70);
        out.addMessage("/pregain/filter",    -70);
        
        out.addMessage("/trigger/perc/drum/trigger",  1);
        out.addMessage("/trigger/perc/drum/motor",  0);
        out.addMessage("/trigger/perc/drum/rate",  0.74);
        out.addMessage("/trigger/perc/drum/buffer/index",  9);
        out.addMessage("/trigger/perc/drum/duration",  -1);
        out.addMessage("/trigger/perc/drum/amp",  1);
        out.addMessage("/trigger/perc/drum/pan",  0);
        out.addMessage("/trigger/perc/drum/xshape",  0);
        out.addMessage("/trigger/perc/drum/xshape",  0);
        out.addMessage("/trigger/perc/drum/yshape",  -5);
        out.addMessage("/trigger/perc/clock/*/ms", 0);

    }
    
    
    // b messages get sent to CV and GL params
    b.addMessage("/video/black",  0);

    if( data.ncontours > 0 )
    {
        size_t nchannels = data.pix_channel_stats[0].size() ;

        if( nchannels )
        {
            nchannels--;

            vector< vector<double> > means;
            means.resize(nchannels);

            for( size_t j = 0; j < nchannels; j++)
            {
                means[j].reserve( data.ncontours );
                for( size_t i = 0; i < data.ncontours; i++)
                {
                    means[j].emplace_back( data.pix_channel_stats[i][j].mean );
                }

            }

            for( size_t j = 0; j < nchannels; j++)
            {
                string addr = "/vec/"+to_string(j);
                out.addMessage(addr, means[j]);

            }

        }
        else
        {
            cout << data.ncontours << " contours but no channels? " << nchannels << endl;

        }

    }

    out.addMessage("/delta_centroid_x", data.delta_centroid_x);
    out.addMessage("/delta_centroid_y", data.delta_centroid_y);
    out.addMessage("/delta_centroid_dist", data.delta_centroid_dist);


    return out;
}
