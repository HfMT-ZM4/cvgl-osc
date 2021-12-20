#include "AnimismCues.hpp"

using namespace cvgl;
using namespace Eigen;

MapOSC cue_black_silent(cueArgs args)
{

    MapOSC out;
    MapOSC &b = args.b;

    if( args.isNewCue )
    {

        out.addMessage("/dpo/pregain/dB",          -100);
        out.addMessage("/dpo/sarah/pregain/dB",    -100);
        out.addMessage("/gran/pregain/dB",         -100);
        out.addMessage("/fuzz/pregain/dB",         -100);
        out.addMessage("/loop/pregain/dB",         -100);
        out.addMessage("/korg/pregain/dB",         -100);
        out.addMessage("/spring/pregain/dB",       -100);
        out.addMessage("/sine/pregain/dB",         -100);

        // video settings
        b.addMessage("/video/black",  1);
        b.addMessage("/use/camera",  1);

        b.addMessage("/overlap/flip", 0.);
        b.addMessage("/overlap/cameras", 0.);

    }

    return out;
}
