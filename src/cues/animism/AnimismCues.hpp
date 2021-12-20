#pragma once

#include "cvglCues.hpp"

MapOSC cue_nothing(cueArgs args);

MapOSC cue_black_silent(cueArgs args);
MapOSC cue_cam1_silent(cueArgs args);

MapOSC cue_cam1_silent(cueArgs args);
MapOSC cue_cam2_silent(cueArgs args);

MapOSC cue_black_click(cueArgs args);

MapOSC cue_cam1_click(cueArgs args);


MapOSC cue_wire_slug(cueArgs args);


MapOSC cue_fog_hands_field(cueArgs args);
MapOSC cue_fog_hands(cueArgs args);
MapOSC cue_origami_bioplastic_mirror(cueArgs args);

MapOSC cue_string_creature(cueArgs args);


MapOSC cue_heart_fadein(cueArgs args);
MapOSC cue_heart_continued(cueArgs args);

MapOSC cue_lungs(cueArgs args);

MapOSC cue_fuzzball_fadein( cueArgs args );
MapOSC cue_fuzzball( cueArgs args );

MapOSC cue_grass(cueArgs args);

MapOSC cue_moon_hands(cueArgs args);

MapOSC cue_antlers( cueArgs args );
MapOSC cue_burdock_roll_out(cueArgs args);

MapOSC cue_pond_edge( cueArgs args );
MapOSC cue_underwater( cueArgs args );
MapOSC cue_underwater_deep( cueArgs args );
MapOSC cue_floating_fisheye_blinks(cueArgs args);


MapOSC cue_forest( cueArgs args );
MapOSC cue_branch_rips_and_falls(cueArgs args);
MapOSC cue_forest_frogs( cueArgs args );
MapOSC cue_forest_loops( cueArgs args );

MapOSC cue_slow_sunburst( cueArgs args );
MapOSC cue_slow_sunburst_noise_start( cueArgs args );
MapOSC cue_slow_sunburst_distortion( cueArgs args );
MapOSC cue_vignette_circle_drop( cueArgs args );

class AnimismCues : public cvglCues
{

public:

    AnimismCues()
    {
        m_cue = "5P";

        setCue("-1",
               "start black",
               "0A - first sound with ensemble",
               &cue_black_silent );

        setScene1();

        // Scene 2 is without camera or electronics
        // (Scene 1 ends black)

        setScene3();
        setScene4();
        setScene5();

    }


    void setScene1()
    {

        setCue("0A",
               "start black click",
               "0B white screen on cam 1",
               &cue_black_click ); // what is the click?

        setCue("0B",
               "camera 1 white + click",
               "",
               &cue_cam1_click ); // what is the click?

        setCue("1A",
               "wire slug on leaf, light from behind, raised up to be more depth angle",
               "leaf-creature",
               &cue_wire_slug ); // *

        setCue("1B",
               "wire slug on leaf, leaf-creatures nibble on head",
               "back to wire material, then pan",
               &cue_wire_slug ); // maybe no change in electronics here?

        setCue("1C",
               "wire slug, then pan to shells",
               "leaves pushing shells",
               &cue_wire_slug );  // maybe no change in electronics here?

        setCue("1D",
               "leaves pushing shells",
               "back to wires",
               &cue_wire_slug ); // maybe no change in electronics here?

        setCue("1E",
               "wire slug on leaf",
               "sync moment with ensemble",
               &cue_wire_slug ); // * new

        setCue("1F",
               "sync moment with ensemble",
               "wire slug on leaf",
               &cue_wire_slug ); //maybe same as 1E

        setCue("1G",
               "wire slug on leaf",
               "slug rips apart",
               &cue_wire_slug ); // back to 1B

        setCue("1H",
               "slug rips apart",
               "cut to cam1, origami all white",
               &cue_wire_slug ); // maybe no change here? cresc in ensemble

        setCue("1I",
               "origami, overexposed & out of focus, gradually focus in, then pan down to goldleaf, then pan to thread on white paper",
               "cut to cam2, leaf surrouded by gel (prepare cam2 during brass/origami section!)",
               &cue_cam1_silent ); // no electronics... just change of camera
// add field recording here?

        setCue("1J",
               "leaf surrouded by gel",
               "curling wave",
               &cue_wire_slug ); // * new

        setCue("1K",
               "curling wave",
               "singing D",
               &cue_wire_slug ); // * change or same as 1J

        setCue("1L",
               "singing D",
               "scene 3 (no electronics or camear in scene 2)",
               &cue_black_silent ); // silent electronics
    }


    void setScene3()
    {
        setCue("3A",
               "start black, shhh",
               "fog-hands (cam2)",
               &cue_black_silent );

        setCue("3B",
               "fog-hands (snares), start black, hands on black, fog, moving very slowly and cloudlike, grandually becoming bigger and bigger",
               "fog-hands",
               &cue_fog_hands_field );

        setCue("3C",
               "fog-hands + strings + field rec? but no plants",
               "cam 1, origami",
               &cue_fog_hands ); // continuing field recording

        setCue("3D", // first cut field recording when brass enters, then fade back in ? or change type of field rec
               "brass section (+ field rec?), cam1 white -> origami -> bioplastic skin / becoming mirrored -> dome thread",
               "percussion enters",
               &cue_origami_bioplastic_mirror ); // transition to double image
// add field recording to scene 1??


        setCue("3E",
               "dome thread (with tuba and 808)",
               "transition to heartbeat",
               &cue_string_creature ); // add field rec?

        setCue("3F",
               "dome thread (with tuba and 808), transitioning to heartbeat",
               "heartbeat, scene 4",
               &cue_string_creature );
    }

    void setScene4()
    {
        setCue("4A",
               "start black, quickly move cam 1 to heart",
               "cut to heart video",
               &cue_black_silent );

        setCue("4B",
               "heart",
               "heart",
               &cue_heart_fadein );

        setCue("4C",
               "heart",
               "heart",
               &cue_heart_continued );

        setCue("4D",
               "heart",
               "heart",
               &cue_heart_continued );

        setCue("4E",
               "heart",
               "heart",
               &cue_heart_continued );

        setCue("4F",
               "desync -> lungs",
               "lungs",
               &cue_lungs );

        setCue("4G",
               "lungs",
               "landscape, scene 5, holding breath",
               &cue_lungs );


    }


    void setScene5()
    {
        setCue("5A",
               "holding breath",
               "fuzzball",
               &cue_fuzzball_fadein );

        setCue("5B",
               "fuzzball",
               "grass creature",
               &cue_fuzzball );

        setCue("5C",
               "grass creature",
               "fuzzball continues on",
               &cue_grass ); // fade out field recording here?

        setCue("5D",
               "fuzzball continues on",
               "moonrise with birds",
               &cue_fuzzball ); // fade field recording back in ?

        setCue("5E",
               "moonrise with birds",
               "antler creature",
               &cue_moon_hands ); // contour filter?

        setCue("5F",
               "antler creature",
               "burdock/corona children roll out",
               &cue_antlers ); // something like grass...

        setCue("5G",
               "burdock/corona children roll out",
               "fuzzball continues on",
               &cue_burdock_roll_out );

        setCue("5H",
               "fuzzball continues on",
               "at water's edge",
               &cue_fuzzball_fadein );

        setCue("5I",
               "at water's edge",
               "underwater",
               &cue_pond_edge ); // with drones? maybe quiet noisy texture drone...
                // respond to light on water?? not sure if this is possible with the camera angle

        setCue("5J",
               "underwater",
               "in the deep black",
               &cue_underwater );

        setCue("5K",
               "in the deep black",
               "floating fish eye blinks",
               &cue_underwater_deep );

        setCue("5L",
               "floating fish eye blinks",
               "descend into the forest",
               &cue_floating_fisheye_blinks );

        // maybe the sun is not too too bright, so that it is more climatic later
        setCue("5M",
               "descend into the forest, sun gets brighter, then wind picks up as we go deeper into the woods",
               "branch rips off and falls",
               &cue_forest );

        setCue("5N",
               "branch rips off and falls (fade in field recording? or add 5Nb for fall?)",
               "frogs and other pond and forest creatures",
               &cue_branch_rips_and_falls );

        setCue("5O",
               "frogs and other pond and forest creatures",
               "natural sounds begin to loop",
               &cue_forest_frogs );

        // slowly fade in different types of mirror images?
        // might work better than pure mirror fragments
        // could also pre-record this part and make hardcore forwards/backwards loops etc.
        setCue("5P",
               "natural sounds begin to loop",
               "slow spectral sunburst",
               &cue_forest_loops );

        // then fade out mirrors when sunburst comes in?
        setCue("5Q",
               "slow spectral sunburst",
               "increasingly dissonant becoming white noise",
               &cue_slow_sunburst );

        setCue("5R",
               "increasingly dissonant becoming white noise",
               "distortion fuzz",
               &cue_slow_sunburst_noise_start );

        setCue("5S",
               "distortion fuzz of ensemble becoming white noise",
               "screen vignette with trumpet solo",
               &cue_slow_sunburst_distortion );

        setCue("5T",
               "screen vignette with trumpet solo, circle falls to ground for screen drop, prepare for puppet performance (electronics end here)",
               "screen drop",
               &cue_vignette_circle_drop );

    }
};
