#include "AnimismCues.hpp"

using namespace cvgl;
using namespace Eigen;

MapOSC cue_grass( cueArgs args )
{

    MapOSC out;
    MapOSC &b = args.b;
    AnalysisData data = args.data;
    const double elapsed_section = args.elapsed_section.count();
    bool isNewCue = args.isNewCue;

    if( isNewCue )
    {
        out.addMessage("/dpo/pregain/dB",          -100);
        out.addMessage("/dpo/sarah/pregain/dB",    0);
        out.addMessage("/gran/pregain/dB",         -100);
        out.addMessage("/fuzz/pregain/dB",         -100);
        out.addMessage("/loop/pregain/dB",         -100);
        out.addMessage("/korg/pregain/dB",         -100);
        out.addMessage("/spring/pregain/dB",       -100);
        out.addMessage("/sine/pregain/dB",         -100);

        b.addMessage("/video/black", 0);
        b.addMessage("/use/preprocess",  0);

        b.addMessage("/size/min", 0.00001 );
        b.addMessage("/size/max", 1 );
        b.addMessage("/thresh", 60 );
        b.addMessage("/invert", 0 );


        b.addMessage("/use/camera",  1);
        b.addMessage("/overlap/cameras", 0.0 );

        b.addMessage("/enable/hull", 0);
        b.addMessage("/enable/minrect", 0);
        b.addMessage("/enable/contour", 0);
        b.addMessage("/contour/color", 0., 0.83, 0.334, 0.2 );

        b.addMessage("/overlap/cameras", 0.);
        b.addMessage("/overlap/flip", 0.);

        out.addMessage("/dpo/intervals", 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
        out.addMessage("/dpo/baseMIDI", 0);

        out.addMessage("/dpo/mod_bus_phase/sync", 0);

        out.addMessage("/dpo/index1/val", 0.38);
        out.addMessage("/dpo/index2/val", 0.68);
        out.addMessage("/dpo/shape/val", 0.29);
        out.addMessage("/dpo/vcf1_q/scale", -2, 0.3);
        out.addMessage("/dpo/vcf2_q/scale", 0, 0.5);
        out.addMessage("/dpo/sarah/slide/up", 0);
        out.addMessage("/dpo/sarah/slide/down", 0);

        out.addMessage("/dpo/amp/val", 1);
        out.addMessage("/dpo/sarah/amp/val", 1);


        out.addMessage("/led/1/spread", 2);
        out.addMessage("/led/1/amp", 1);
        out.addMessage("/led/rand/pos/hz", 2.5);
        out.addMessage("/led/rand/pos/range", 29, 88);
        out.addMessage("/led/rand/pos/snapshot", 100);


        args.cache.addMessage("/min", 1);
        args.cache.addMessage("/max", 0);

        args.cache.addMessage("/dmin", 1);
        args.cache.addMessage("/dmax", 0);

        args.cache.addMessage("/prev_t", elapsed_section);


    }


  //  out.addMessage("/data/start", data.start_time);


    if( data.ncontours > 0 && data.delta_centroid_dist.size() > 0 )
    {

        out.addMessage("/data/delta", data.delta_centroid_dist);
        out.addMessage("/data/dur", data.elapsed_contour);


        // adaptive range

        double range_reset_s = 10;

        double area_sum = data.contour_area.sum();
        double dist_sum = data.delta_centroid_dist.sum();

        double min = args.cache["/min"].getFloat();
        double max = args.cache["/max"].getFloat();

//            double this_min = data.contour_area.minCoeff();
//            double this_max = data.contour_area.maxCoeff();

        min = min > area_sum ? area_sum : min;
        max = max < area_sum ? area_sum : max;


        double dmin = args.cache["/dmin"].getFloat();
        double dmax = args.cache["/dmax"].getFloat();

    //    double this_dmin = data.delta_centroid_dist.minCoeff();
    //    double this_dmax = data.delta_centroid_dist.maxCoeff();

        dmin = dmin > dist_sum ? dist_sum : dmin;
        dmax = dmax < dist_sum ? dist_sum : dmax;


        if( (elapsed_section - args.cache["/prev_t"].getFloat()) > range_reset_s )
        {
//                min = 1;
//                max = 0;
            dmin = 1;
            dmax = 0;
            args.cache.addMessage("/prev_t", elapsed_section);
        }

        args.cache.addMessage("/min", min);
        args.cache.addMessage("/max", max);
        args.cache.addMessage("/dmin", dmin);
        args.cache.addMessage("/dmax", dmax);




     //   double centroid_dist_avg = data.delta_centroid_dist.mean();
     //   double area_avg = data.contour_area.mean();

        out.addMessage("/dpo/f2/val", scale(dist_sum, 0, 1, 135, 50), 20);
        out.addMessage("/dpo/index1/val", scale(area_sum, 0, 1, 0.4, 0.8) + scale(dist_sum, 0, 1, 0.2, 0.8), 20);

        out.addMessage("/dpo/sarah/amp/val", dist_sum * dist_sum  ); // area_sum * area_sum + scale(dist_sum, dmin, dmax, 0.00, 1 - area_sum)

        out.addMessage("/dpo/vcf1_hz/val", scale(area_sum, min, max, -0.3, 0));
        out.addMessage("/dpo/vcf2_hz/val", scale(area_sum, min, max, 0, 0.8));

        out.addMessage("/dpo/vcf1_q/val", scale(area_sum, min, max, -1, 0.8));
        out.addMessage("/dpo/vcf2_q/val", scale(area_sum, min, max, -1, 0.8));


    }
    else
    {
        out.addMessage("/dpo/sarah/amp/val", 0);
    }


    out.addMessage("/cache", args.cache);

    return out;
}
