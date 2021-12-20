#include "AnimismCues.hpp"

using namespace cvgl;
using namespace Eigen;

MapOSC cue_moon_hands(cueArgs args)
{
    MapOSC out;
    MapOSC &b = args.b;
    AnalysisData data = args.data;
    const double elapsed_section = args.elapsed_section.count();
    bool isNewCue = args.isNewCue;

    if( isNewCue )
    {

        out.addMessage("/dpo/pregain/dB",          -100);
        out.addMessage("/dpo/sarah/pregain/dB",    -100);
        out.addMessage("/gran/pregain/dB",         -100);
        out.addMessage("/fuzz/pregain/dB",         -100);
        out.addMessage("/loop/pregain/dB",         -100);
        out.addMessage("/korg/pregain/dB",         -100);

        b.addMessage("/video/black",  0);

        b.addMessage("/use/camera",  2);
        b.addMessage("/overlap/cameras", 0.0 );

        b.addMessage("/enable/hull", 0);
        b.addMessage("/enable/minrect", 0);
        b.addMessage("/enable/contour", 0);
        b.addMessage("/contour/color", 0.25, 0.5, 1., 0.125 );

        b.addMessage("/overlap/cameras", 0.);
        b.addMessage("/overlap/flip", 0.);

        b.addMessage("/use/preprocess",  0);
      //  cout << "use camera" << 2 << endl;
        b.addMessage("/size/min", 0.000 );
        b.addMessage("/size/max", 0.9 );
        b.addMessage("/thresh", 41 );
        b.addMessage("/invert", 0 );

        out.addMessage("/loop/amp", 1);
        out.addMessage("/loop/length/ms", -1);
        out.addMessage("/loop/retrigger/enable", 0);
        out.addMessage("/loop/start/ratio", 0);
        out.addMessage("/loop/transpose", 0);
        out.addMessage("/loop/buffer/idx", 0);
        out.addMessage("/loop/retrigger/click", 1);


    }

    double fadetime = 30;
    if( elapsed_section <= fadetime )
    {
        // transition, to sped up version
        out.addMessage("/loop/pregain/dB",  scale(elapsed_section, 0., fadetime, -6., 0) );
        out.addMessage("/loop/transpose",   scale(elapsed_section, 0., fadetime, 100., 0) );
    }


    out.addMessage("/elapsed_section", elapsed_section );

    double area_sum = data.contour_area.sum();
    out.addMessage("/contour_area_sum", area_sum );


    return out;

}
