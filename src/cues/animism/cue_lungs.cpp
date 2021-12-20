#include "AnimismCues.hpp"

using namespace cvgl;
using namespace Eigen;

MapOSC cue_lungs(cueArgs args)
{

    MapOSC out;
    MapOSC &b = args.b;
    AnalysisData data = args.data;
    const double elapsed_section = args.elapsed_section.count();
    bool isNewCue = args.isNewCue;
    MapOSC& state_cache = args.cache;

    if( isNewCue )
    {

        out.addMessage("/dpo/pregain/dB",          -100);
        out.addMessage("/dpo/sarah/pregain/dB",    -100);
        out.addMessage("/loop/pregain/dB",         -100);
        out.addMessage("/korg/pregain/dB",         0);
        out.addMessage("/gran/pregain/dB",         0);
        out.addMessage("/fuzz/pregain/dB",         0);

        out.addMessage("/korg/maths/cycle", 1);

        out.addMessage("/korg/amp", 0);
        out.addMessage("/fuzz/amp", 0);

        b.addMessage("/video/black",  0);
        b.addMessage("/use/preprocess",  3);

        b.addMessage("/use/camera",  2);
        b.addMessage("/overlap/cameras", 0.5 );

        b.addMessage("/enable/hull", 0);
        b.addMessage("/enable/minrect", 0);
        b.addMessage("/enable/contour", 1);
        b.addMessage("/contour/color", 1, 0., 0., 0.01 );

       // cout << "use camera" << 2 << endl;
        b.addMessage("/size/min", 0.000 );
        b.addMessage("/size/max", 0.9 );
        b.addMessage("/thresh", 41 );
        b.addMessage("/invert", 0 );

        out.addMessage("/loop/length/ms", -1);
        out.addMessage("/loop/retrigger/enable", 0);
        out.addMessage("/loop/start/ratio", 0);
        out.addMessage("/loop/transpose", 24);
        out.addMessage("/loop/buffer/idx", 0);

        out.addMessage("/gran/*/amp/val", 0);

        out.addMessage("/gran/1/motor/val", 84.);
        out.addMessage("/gran/1/overlap/val", 1);
        out.addMessage("/gran/1/rate/val", 0.2);
        out.addMessage("/gran/1/amp/val", 1);
        out.addMessage("/gran/1/buffer/val", 7);
        out.addMessage("/gran/send", 1);

        out.addMessage("/fuzz/drive/val", 0.5);
        out.addMessage("/fuzz/fat/val", 0.);
        out.addMessage("/fuzz/impedance/val", 0.13);
        out.addMessage("/fuzz/comp/val", 0.); //0.4
        out.addMessage("/fuzz/stab/val", 0.64);
        out.addMessage("/fuzz/gate/val", 0.);
        out.addMessage("/fuzz/pan/val", 0.);
        out.addMessage("/fuzz/amp", 1);

        state_cache.addMessage("/min", 1);
        state_cache.addMessage("/max", 0);
        state_cache.addMessage("/prev_t", elapsed_section);


    }

    out.addMessage("/gran/1/use/position", 1);


    out.addMessage("/elapsed_section", elapsed_section );
    double area_sum = data.contour_area.sum();
    out.addMessage("/contour_area_sum", area_sum );

    double sum_mag = 0, sum_area = 0, sum_x = 0, sum_y = 0;

    if( data.ncontours > 0 )
    {
        for( size_t i = 0 ; i < data.ncontours; i++ )
        {
            double w = data.contour_area[i];
            sum_area += w;

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

        //if( sum_mag > 0 )
        {
            double scalar = sum_area == 0 ? 1 : sum_area;
            double mag_avg = sum_mag / scalar;

            double avg_dist_x = sum_x / scalar;
            double avg_dist_y = sum_y / scalar;

            double norm_mag_avg = mag_avg / 255. ;
            double norm_2 = clip( pow( norm_mag_avg, exp(1.5)) * 100, 0., 1.);



            double min = state_cache["/min"].getFloat();
            double max = state_cache["/max"].getFloat();

            min = min > norm_mag_avg ? norm_mag_avg : min;
            max = max < norm_mag_avg ? norm_mag_avg : max;

            if( (elapsed_section - state_cache["/prev_t"].getFloat()) > 10 )
            {
                min = 1;
                max = 0;
                state_cache.addMessage("/prev_t", elapsed_section);
            }

            state_cache.addMessage("/min", min);
            state_cache.addMessage("/max", max);

            double adaptive_norm_mag_avg = scale(norm_mag_avg, min, max, 0., 1.);
            out.addMessage("/data/adaptive_norm_mag_avg", adaptive_norm_mag_avg);



            out.addMessage("/avg/mag", mag_avg );
            out.addMessage("/avg/x", avg_dist_x);
            out.addMessage("/avg/y", avg_dist_y );


            out.addMessage("/gran/1/motor/val", scale( abs(avg_dist_x), 0., 10,  20, 250));
            out.addMessage("/gran/1/amp/val", norm_2 * 0.5 );


            if( avg_dist_y < 0 ) // up
            {
                out.addMessage("/gran/1/rate/val", clip( scale( abs(avg_dist_y), 0., 8,  0.2, 0.1), 0.1, 0.2) );
                out.addMessage("/gran/1/overlap/val", clip( scale( abs(avg_dist_x), 0., 8,  20, 100), 0.1, 100) );

                out.addMessage("/korg/amp", norm_2);
                out.addMessage("/korg/slide/down", 100);
                out.addMessage("/korg/slide/up", 5);
                out.addMessage("/korg/q1/val", 0.65 );
                out.addMessage("/korg/q2/val", 0.68 );
                out.addMessage("/korg/maths/speed/val", scale(norm_mag_avg, 0., 0.5, 0.1, -0.8) );
                out.addMessage("/korg/maths/speed/smooth", 100 ); // adjusted for 32 vector size in max
                out.addMessage("/korg/maths/offset/val", scale(pow( norm_mag_avg, exp(-0.5)), 0., 1., 0, 1));


                //out.addMessage("/gran/1/overlap/val", 20);

                out.addMessage("/fuzz/drive/val", 0.5);
                out.addMessage("/fuzz/fat/val", 0.);
                out.addMessage("/fuzz/stab/val", clip( scale( abs(avg_dist_y), 0., 8,  0.3, 0.5), 0.3, 0.5) );
                out.addMessage("/fuzz/amp", norm_2 );

                out.addMessage("/loop/transpose", 36);
            }
            else
            {

                norm_2 = clip( pow( norm_mag_avg, exp(1.5)) * 150, 0., 1.);

                out.addMessage("/gran/1/rate/val", 0.13);
                out.addMessage("/gran/1/overlap/val", 0.1);

                out.addMessage("/fuzz/drive/val", 0.);
                out.addMessage("/fuzz/fat/val", 1);
                out.addMessage("/fuzz/stab/val", clip( scale( abs(avg_dist_y), 0., 8,  0.5, 0.6), 0.5, 0.6) );
                out.addMessage("/fuzz/amp", norm_2 * 0.5 );

                out.addMessage("/korg/amp", norm_2);
                out.addMessage("/korg/slide/down", 100);
                out.addMessage("/korg/slide/up", 5);
                out.addMessage("/korg/q1/val", 0.65 );
                out.addMessage("/korg/q2/val", 0.68 );
                out.addMessage("/korg/maths/speed/val", scale(norm_mag_avg, 0., 0.5, 0.1, -0.8) );

//                    out.addMessage("/korg/maths/speed/val", clip( scale( abs(avg_dist_y), 0., 8,  -0.8, 1), -0.8, 1) );
                out.addMessage("/korg/maths/speed/smooth", 100 ); // adjusted for 32 vector size in max
                out.addMessage("/korg/maths/offset/val", scale(pow( norm_mag_avg, exp(-0.5)), 0., 1., -0.6, 0));

                out.addMessage("/loop/transpose", 24);

            }


        }


    }
    else
    {
        out.addMessage("/fuzz/amp", 0 );
        out.addMessage("/gran/1/amp/val", 0, 200 );
        out.addMessage("/korg/amp", 0 );


    }

    return out;
}
