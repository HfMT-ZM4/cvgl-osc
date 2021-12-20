#include "AnimismCues.hpp"

using namespace cvgl;
using namespace Eigen;

MapOSC cue_origami_bioplastic_mirror(cueArgs args)
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
        out.addMessage("/loop/pregain/dB",         0);
        out.addMessage("/korg/pregain/dB",         -100);

        b.addMessage("/video/black",  0);

        b.addMessage("/use/camera",  1);
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
    }

    double fadetime = 30;
    if( elapsed_section <= fadetime )
    {
        out.addMessage("/loop/pregain/dB",  scale_clip(elapsed_section, 0., 10, 0, -12) );
        b.addMessage("/overlap/flip", scale_clip(elapsed_section, 10., fadetime, 0., 0.5));

    }



    return out;

}
