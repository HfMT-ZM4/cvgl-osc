#include "AnimismCues.hpp"

using namespace cvgl;
using namespace Eigen;

MapOSC cue_vignette_circle_drop( cueArgs args )
{

    MapOSC out;
    MapOSC &b = args.b;
    AnalysisData data = args.data;
    const double elapsed_section = args.elapsed_section.count();
    bool isNewCue = args.isNewCue;

    if( isNewCue )
    {

        b.addMessage("/use/camera",  1);
        b.addMessage("/video/black", 0);

        b.addMessage("/enable/hull", 0);
        b.addMessage("/enable/minrect", 0);
        b.addMessage("/enable/contour", 0);

        b.addMessage("/vignette/xyr", 0.5, 0.5, 0.5 );

    }


    double dur1 = 45;
    double dur2 = 30 + dur1;
    //double dur3 = 10 + dur2;

    if( elapsed_section <= dur1 )
    {

        double t = scale_clip( elapsed_section, 0, dur1, 0., 1. );
        double ease = easeInOutSine(t);
        float rad = scale(ease, 0, 1, 1, 0.1);
        float x =   0.5;
        float y =   scale(ease*ease*ease*ease*ease, 0, 1, 0.5, 0.185); //0.185

        b.addMessage("/vignette/xyr", x, y, rad );
        out.addMessage("/rad", rad);
        out.addMessage("/y", y);

    }
    else if( elapsed_section <= dur2 )
    {

        double t = scale_clip( elapsed_section, dur1, dur2, 0., 1. );
        double ease = easeInSine(t);

        float rad = scale(ease*ease, 0, 1, 0.1, 2) ;
        float x =   0.5;
        float y =   scale(ease, 0, 1, 0.185, 3.085); //1.2

        b.addMessage("/vignette/xyr", x, y, rad );
    }
    /*
    else if( elapsed_section <= dur2 )
    {

        double t = scale_clip( elapsed_section, dur1, dur2, 0., 1. );
        double ease = easeInOutSine(t);

        float rad = 0.1 ;
        float x =   0.5;
        float y =   scale(ease, 0, 1, 0.185, 0.9); //1.2

        b.addMessage("/vignette/xyr", x, y, rad );
    }
    else if( elapsed_section <= dur3 )
    {

        double t = scale_clip( elapsed_section, dur2, dur3, 0., 1. );
        double ease = easeInOutSine(t);

        float rad = scale(ease, 0, 1, 0.1, 0.9) ;
        float x =   0.5;
        float y =   scale(ease, 0, 1, 0.9, 1.8);

        b.addMessage("/vignette/xyr", x, y, rad );
    }
    */

    return out;

}
