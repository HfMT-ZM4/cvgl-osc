#include "AnimismCues.hpp"

using namespace cvgl;
using namespace Eigen;

MapOSC cue_string_creature(cueArgs args)
{
    MapOSC out;
    MapOSC &b = args.b;
    AnalysisData data = args.data;
    MapOSC& cache = args.cache;
    const double elapsed_section = args.elapsed_section.count();
    bool isNewCue = args.isNewCue;

    cvglRandom& rand_generator = args.randGen;

    if( isNewCue )
    {

        out.addMessage("/gran/pregain/dB",         0);
        out.addMessage("/gran/*/amp/val", 0);

        out.addMessage("/fuzz/pregain/dB",         -100);
        out.addMessage("/loop/pregain/dB",         -100);
        out.addMessage("/korg/pregain/dB",         -100);
        out.addMessage("/spring/pregain/dB",       -100);
    //    out.addMessage("/sine/pregain/dB",         -40);

        b.addMessage("/video/black",  0);
        b.addMessage("/use/preprocess",  3);

        b.addMessage("/use/camera",  1);
        b.addMessage("/overlap/cameras", 0.0 );
        b.addMessage("/overlap/flip", 0.5);

        b.addMessage("/enable/hull", 0);
        b.addMessage("/enable/minrect", 0);
        b.addMessage("/enable/contour", 1);
        b.addMessage("/contour/color", 1, 0., 1, 0.25 );

      //  cout << "use camera" << 2 << endl;
        b.addMessage("/size/min", 0.000 );
        b.addMessage("/size/max", 0.9 );
        b.addMessage("/thresh", 20 );
        b.addMessage("/invert", 0 );

        out.addMessage("/loop/length/ms", -1);
        out.addMessage("/loop/retrigger/enable", 0);
        out.addMessage("/loop/start/ratio", 0);
        out.addMessage("/loop/transpose", 24);
        out.addMessage("/loop/buffer/idx", 0);

        out.addMessage("/gran/*/motor/scale", 0.001, 500.);
        out.addMessage("/gran/*/overlap/scale", 0.01, 0.8);
        out.addMessage("/gran/*/rate/scale", 15., 117.);
        out.addMessage("/gran/*/amp/val", 0);
        out.addMessage("/gran/*/buffer/scale", 3, 9 );
        out.addMessage("/gran/send", 1);
        cache.addMessage("/position", 0);
        cache.addMessage("/direction", 1);
        cache.addMessage("/min", 1);
        cache.addMessage("/max", 0);

        cache.addMessage("/prev_t", elapsed_section);

    }


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

        if( sum_mag > 0 )
        {
            double scalar = sum_area == 0 ? 1 : sum_area;
            double mag_avg = sum_mag / scalar;

            double avg_dist_x = sum_x / scalar;
            double avg_dist_y = sum_y / scalar;

            double norm_mag_avg = mag_avg / 255.0 ;
            double norm_2 = clip( pow( norm_mag_avg, exp(-0.5)) * 100, 0.1, 1.);
            double norm_22 = clip( pow( norm_mag_avg, exp(1.5)) * 100, 0.1, 1.);

            out.addMessage("/data/norm_2", norm_2);
            out.addMessage("/data/norm_mag_avg", norm_mag_avg);

            double min = cache["/min"].getFloat();
            double max = cache["/max"].getFloat();

            min = min > norm_mag_avg ? norm_mag_avg : min;
            max = max < norm_mag_avg ? norm_mag_avg : max;

            if( (elapsed_section - cache["/prev_t"].getFloat()) > 10 )
            {
                min = 1;
                max = 0;
                cache.addMessage("/prev_t", elapsed_section);
            }

            cache.addMessage("/min", min);
            cache.addMessage("/max", max);

            double adaptive_norm_mag_avg = scale(norm_mag_avg, min, max, 0., 1.);
            out.addMessage("/data/adaptive_norm_mag_avg", adaptive_norm_mag_avg);

            double pos = cache["/position"].getFloat();
            double direction = cache["/direction"].getFloat();
            double step = direction * norm_2 * 0.1;

            if( abs(step) > 0.01 )
            {
                double next = pos + step;

                if( next > 1 ||  next < 0 )
                {
                    step *= -1;
                    cache.addMessage("/direction", direction * -1);

                    next = pos + step;
                }

                pos = next;

                cache.addMessage("/position", pos);

            }


            out.addMessage("/gran/1/position", pos, 100 );
            out.addMessage("/gran/3/position", pos, 100 );

            double thresh = 0.3;
            if( adaptive_norm_mag_avg > thresh )
            {
                out.addMessage("/gran/3/amp/val", adaptive_norm_mag_avg, 20);
                out.addMessage("/gran/3/buffer/val", 9 );
                out.addMessage("/gran/3/overlap/val", scale(adaptive_norm_mag_avg, thresh, 1., 0.01, 1), 5);
                out.addMessage("/gran/3/motor/val", scale(adaptive_norm_mag_avg, thresh, 1., 20, 100) );
                out.addMessage("/gran/3/rate/val",   scale(adaptive_norm_mag_avg, thresh, 1., 1, 0.5) );


                out.addMessage("/gran/2/play", 1);
                out.addMessage("/gran/2/retrigger", 0);
                out.addMessage("/gran/2/ms", 4000);
                out.addMessage("/gran/2/loop", 0);
                out.addMessage("/gran/2/amp/val", 0 );
                out.addMessage("/gran/2/motor/scale", 0.07, 471.);


            }
            else
            {
//                    out.addMessage("/gran/1/amp/val", norm_2, 20);
            }

            out.addMessage("/gran/1/amp/val", adaptive_norm_mag_avg, 20);
            out.addMessage("/gran/1/rate/val", scale(adaptive_norm_mag_avg, 0., 1., 4, 0.1) );
            out.addMessage("/gran/1/buffer/val", 0 );
            out.addMessage("/gran/1/overlap/val", scale(adaptive_norm_mag_avg, 0., 1., 1, 2));
            out.addMessage("/gran/1/motor/val", scale(adaptive_norm_mag_avg, 0., 1., 20, 200));


        }


    }
    else
    {
      //  out.addMessage("/gran/1/position", 0, 1000 );
        out.addMessage("/gran/1/amp/val", 0, 20);
        out.addMessage("/gran/2/amp/val", 0, 200);
        out.addMessage("/gran/3/amp/val", 0, 20);
        out.addMessage("/gran/3/motor/scale", 0.0001, 20);

        out.addMessage("/gran/2/motor/scale", 10 + rand_generator.uniformRand() * 10, 471.);

    }



    out.addMessage("/cache", cache);

    return out;
}
