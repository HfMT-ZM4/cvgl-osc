#include "AnimismCues.hpp"

using namespace cvgl;
using namespace Eigen;

MapOSC cue_forest_loops( cueArgs args )
{

    MapOSC out;
    MapOSC &b = args.b;
    AnalysisData data = args.data;
    const double elapsed_section = args.elapsed_section.count();
    bool isNewCue = args.isNewCue;

    //MapOSC& m_state_cache = args.cache;
    //cvglRandom& m_rand_generator = args.randGen;

    double fadetime = 20;
    if( elapsed_section <= fadetime )
    {
        // transition, to sped up version
       // out.addMessage("/loop/pregain/dB",  scale(elapsed_section, 0., fadetime, 0., -12));
        out.addMessage("/korg/pregain/dB",  scale(elapsed_section, 0., fadetime, -70., -6));
        out.addMessage("/loop/transpose",   scale(elapsed_section, 0., fadetime, 0., 12) );

      //  b.addMessage("/big_triangle/x", scale( cvgl::easeInOutSine( scale(elapsed_section, 0., fadetime, 0., 1.)),0., 1., -1, 0) );
        b.addMessage("/big_triangle/alpha", cvgl::easeInSine( scale(elapsed_section, 0., 10, 0., 1.)) );
        b.addMessage("/big_triangle2/alpha", cvgl::easeInSine( scale_clip(elapsed_section, 0., fadetime, 0., 0.75)) );

    }

    if( isNewCue )
    {

        out.addMessage("/dpo/pregain/dB",          -100);
        out.addMessage("/dpo/sarah/pregain/dB",    -100);
        out.addMessage("/gran/pregain/dB",         -100);
        out.addMessage("/fuzz/pregain/dB",         -100);

        out.addMessage("/loop/pregain/dB",         0);
        out.addMessage("/korg/pregain/dB",         -70);

        out.addMessage("/sine/pregain/dB",  -70);

        b.addMessage("/video/black",  0);

        b.addMessage("/use/camera",  1);

        b.addMessage("/overlap/cameras", 0.);
        b.addMessage("/overlap/flip", 0.);

        b.addMessage("/enable/hull", 0);
        b.addMessage("/enable/minrect", 0);
        b.addMessage("/enable/contour", 0);
        b.addMessage("/contour/color", 0.25, 0.5, 1., 0.125 );

      //  cout << "use camera" << 2 << endl;
        b.addMessage("/use/preprocess", 3);
        b.addMessage("/size/min", 0.0001 );
        b.addMessage("/size/max", 0.9 );
        b.addMessage("/thresh", 10.0 );
        b.addMessage("/invert", 0 );

        out.addMessage("/loop/length/ms", -1);
        out.addMessage("/loop/retrigger/enable", 0);
        out.addMessage("/loop/start/ratio", 0);
        out.addMessage("/loop/transpose", 0);
        out.addMessage("/loop/buffer/idx", 0);

        out.addMessage("/korg/maths/cycle", 1);

    }

    out.addMessage("/elapsed_section", elapsed_section );
    double area_sum = data.contour_area.sum();
    out.addMessage("/contour_area_sum", area_sum );

    double sum_mag = 0, sum_area = 0, sum_x = 0, sum_y = 0;

    double avg_x = 0;

    if( data.ncontours > 0 )
    {
        for( size_t i = 0 ; i < data.ncontours; i++ )
        {
            double w = data.contour_area[i];
            sum_area += w;

            avg_x += data.center_x[i] * w;

            if( data.pix_channel_stats[i].size() == 5 ) // focus channel is added to whatever the channel count is
            {
           //     sum_angle += data.pix_channel_stats[i][0].mean * w;
                sum_mag += data.pix_channel_stats[i][1].mean * w;
                sum_x += data.pix_channel_stats[i][2].mean * w;
                sum_y += data.pix_channel_stats[i][3].mean * w;
            }
            else
            {
                out.addMessage("/nchannels", (int32_t)i, (int32_t)data.pix_channel_stats[i].size() );
            }


        }

        out.addMessage("/flow/x", sum_x);
        out.addMessage("/flow/y", sum_y);

       // b.addMessage("/offset/triangles/x", sum_x * 0.1);

    }
    else
    {

    }

    b.addMessage("/triangle/interact/x", data.centroid_x);

    return out;
}
