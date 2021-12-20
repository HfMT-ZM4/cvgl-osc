#include "cvglCues.hpp"
// b messages get sent to internal CV and GL params
// #include "cvglProfile.hpp"

// >> Need to have limits on number of voices below -- the UDP port is getting blocked when the size is too big

MapOSC cvglCues::cue1( const AnalysisData& data, MapOSC& b )
{
    using namespace cvgl;
    
    MapOSC out;
    out.addMessage("/descr", "black and feather, venus flytrap spider");
    
    const double elapsed_section = m_elapsed_section.count();
    
    
    if( isNewCue )
    {
        out.addMessage("/pregain/perc",      -12);
        out.addMessage("/pregain/gran",      -3);
        out.addMessage("/pregain/res",       -70);
        out.addMessage("/pregain/filter",    -21);
     
        out.addMessage("/filter/field/direct/amp", 0);


    }

    b.addMessage("/video/black",  0);
    b.addMessage("/use/preprocess",  2);
    
    b.addMessage("/size/min", 0.000 );
    b.addMessage("/size/max", 0.9 );
    b.addMessage("/thresh", 16 );
    b.addMessage("/invert", 0 );

    
    // figure out gl and cv settings here

    /*
     /draw/hull/type = "polygon",
     /draw/hull/color = [1,1,1,0.01],

     /draw/minrect/color = [1., 1, 1., 0.1],
     /draw/defects/color = [1, 1, 1, 0.03],

     /cv/attr/resize_scale = 0.25,

     /cv/attr/track_radius = 0.1,
     /cv/attr/invert = 0,
     /cv/attr/thresh = 8,

     /cv/attr/min_size = 0.009,
     /cv/attr/max_size = 0.9,

     /cv/attr/gauss_sigma = 3,
     /cv/attr/erosion = 0,
     /cv/attr/dilation = 0,
     */
    
    if( data.ncontours == 0 ){
        out.addMessage("/perc/clock/*/scale/amp", 0., 0.);
        return out;
    }
    
    
    size_t new_contours = data.noteOn_idx.size();
    int32_t contour_incr = !m_state_cache.addressExists("/contour_incr") || isNewCue ? 0 : m_state_cache["/contour_incr"][0].get<int32_t>();
    
    contour_incr += new_contours;
    // (same as noteon/incr in cues)
    
    
    m_state_cache.addMessage("/contour_incr", contour_incr);
    
    vector< vector<double> > interval_set;
    
    interval_set.emplace_back(vector<double>{0, 12+2, 2, 12+3.86, 3.86, 12+7, 10.86});
    interval_set.emplace_back(vector<double>{0, 24+1, 2, 24+3.16, 3.86, 12+7, -10.86});
    interval_set.emplace_back(vector<double>{0, 2, 3.86});
    
    double seq_t = fmod( clip( elapsed_section, 0., 120.), 20.);

    vector<double> transp_seq_v(    {-1,     2,      0,      -2,     -2});
    vector<string> transp_seq_tonic({"eb:3", "f:3",  "e:3",  "d:3",  "d:3"});
    vector<int32_t> set_seq(        {0,       1,      2,      1,      1});
    
    ArrayXd transp_seq_t(5);
    transp_seq_t <<                  0,       4,      8,      10,     20;
    

    size_t seq_minIdx = (seq_t >= transp_seq_t ).sum() - 1;
    size_t seq_maxIdx = transp_seq_t.size() - ( seq_t < transp_seq_t ).sum();

    double tonic = ntom( transp_seq_tonic[ seq_minIdx ] );
    
    vector<double> intervals = interval_set[ seq_minIdx ];


    // perc
    
    // fade region (all are 0 0 1 1 in plastisphere...)
    ArrayXd perc_amp = data.contour_area.unaryExpr([](double x){ return (double)(x > 0.); });
    out.addMessage("/perc/amp", perc_amp );

    /*
    // option 2 for using lookup line for fade region
    ArrayXd amps = lineLookup( data.contour_area, {0, 0.2, 0.5, 0.6},
                                                  {0, 1,   0.5, 0} );
    */
     
    
    if( isNewCue )
    {
        out.addMessage("/perc/drum/trigger", 1);
        out.addMessage("/perc/drum/motor", 0);
        out.addMessage("/perc/drum/rate", 1);
        out.addMessage("/perc/drum/duration", -1);
        out.addMessage("/perc/drum/amp", 1);
        out.addMessage("/perc/drum/buffer/index", 12);
        out.addMessage("/perc/drum/pan", 0.);
        // update to beta window?
        out.addMessage("/perc/drum/xshape", 0);
        out.addMessage("/perc/drum/yshape", -5);

    }
    
    for( const auto& i : data.noteOn_idx )
    {
        out.addMessage("/perc/clock/"+ to_string( data.id[i] + 1 ) +"/trigger", 1);
    }
    
    for( size_t i = 0; i < data.ncontours; ++i )
    {
        double scaled = clip( data.contour_area[i] * 10., 0., 1.);
      //  cout << scaled << " ";
        double time_out = scaled;// - clip( data.elapsed_contour[i] * 0.1, 0., 1.);
        string idx = to_string(i+1);
        
        out.addMessage("/perc/clock/"+idx+"/position", scaled, scaled * 100. );
        out.addMessage("/perc/clock/"+idx+"/phraseidx", 0 );
        out.addMessage("/perc/clock/"+idx+"/scale/index", 0., 14.);
        out.addMessage("/perc/clock/"+idx+"/scale/motor", (scaled == 1 ? 20 : 0.001),  500. );
        out.addMessage("/perc/clock/"+idx+"/scale/dur", 1, 100. );
        out.addMessage("/perc/clock/"+idx+"/scale/rate", 1. / ( scaled * 7 ), scaled * 10. );
        out.addMessage("/perc/clock/"+idx+"/scale/amp", 0., time_out );
        out.addMessage("/perc/clock/"+idx+"/scale/pan", scale( data.centroid_x[i], 0., 1., -1., 1) );
        out.addMessage("/perc/clock/"+idx+"/scale/reverb/sendB", -60., -6.);
    }
  //  cout << endl;;
  //  cout << tonic << endl;
    
    // Gran
    
    // fade region (all are 0 0 1 1 in plastisphere...)
    ArrayXd gran_amps = (data.contour_area > 0).cast<double>();
    out.addMessage("/gran/amp", gran_amps );


    out.addMessage("/gran/fadein/ms", 20);
    out.addMessage("/gran/fadeout/ms", 500);//, #let ring

    vector<double> scale_y = intervals;
    
    vector<int> scale_x = cvgl::aseq<int>( (int)0, (int)scale_y.size()-1 );
    
    double oct = (double)scale_x.size();
    
    vector< LookupFigure > figure(4);
    
    
    figure[0] = LookupFigure( {0}, {0});
    
    vector<double> fig_1_y = { oct+1, -oct,    0 };
    vector<double> fig_1_x;

    if( isNewCue )
    {
        for( size_t i = 0; i < fig_1_y.size(); ++i )
        {
            fig_1_x.emplace_back( m_rand_generator.uniformRand() );
        }
        
        sort( fig_1_x.begin(), fig_1_x.end() );
        
//        fig_1_x = scale<double>(fig_1_x, fig_1_x.front(), fig_1_x.back(), 0, 8);
        
        std::for_each(fig_1_x.begin(), fig_1_x.end(), [&](double &n){
            n = cvgl::round( scale(n, fig_1_x.front(), fig_1_x.back(), 0, 8 ) ); });

        m_state_cache.addMessage("/fig_1_x", fig_1_x);
        
    }
    else
    {
        for( auto& at : m_state_cache["/fig_1_x"].getAtomVector() )
        {
            fig_1_x.emplace_back( at->get<double>() );
        }
    }
    
    
    figure[1] = LookupFigure(fig_1_x, fig_1_y);
    
    figure[2] = LookupFigure({0, 4,  5},
                             {0, 1,  2});

    figure[3] = LookupFigure({0, 4,  5, 8, 9, 10},
                             {0, 1,  2, 3, 4, 5});
    


    vector<int32_t> steps{ 0,  1, 2,  3 }; // #offset for figure
        
    //  weighted random selections
    vector<double> weights{      0.5,  0.01,  0.9,  0,   0,   0};
    
    ArrayXi octave_opt(6);
    octave_opt <<                6,    -2,    6,   6,   6,   7;
    octave_opt *= 12;
    
    vector<double> dur_env_opt{  1,    500,   0.5,  200, 200, 300 };
    vector<double> dur_beat_opt{ 10,   1000,  100,  50,  200, 100};
    vector<double> fig_opt{      0,    0,     3,    2,   1,   0};
    vector<double> amp_mul_opt{  1,    1,     1,    1,   1,   1};
    vector<double> xshape_opt{  -5,   -10,   -5,    -5,   -5,  -5};
    vector<double> gran_rev_opt{ 0,    0.05,  0,    0,    0,   0};

    m_rand_generator.setWeightedDist(weights);

    int32_t incr = contour_incr % steps.size();
    
    // this still happens even when there are no contours...
    for( const auto& id_idx : data.id_idx )
    {
        string id = to_string( id_idx.first + 1 );
        size_t idx = id_idx.second;
        
        size_t weightedsel = m_rand_generator.discreteDist();
        size_t sel_fig = fig_opt[weightedsel];

        out.addMessage( "/gran/"+id+"/scale/x", scale_x );
        out.addMessage( "/gran/"+id+"/scale/y", scale_y );
        out.addMessage( "/gran/"+id+"/baseNote", tonic + octave_opt(weightedsel) );
        out.addMessage( "/gran/"+id+"/step",  steps[ incr ] );
        out.addMessage( "/gran/"+id+"/fig/x", figure[sel_fig].x );
        out.addMessage( "/gran/"+id+"/fig/y", figure[sel_fig].y );
        out.addMessage( "/gran/"+id+"/dur", dur_env_opt[ weightedsel ] );
        out.addMessage( "/gran/"+id+"/phrase/dur", figure[sel_fig].x.size() * dur_env_opt[ weightedsel ] );
        out.addMessage( "/gran/"+id+"/xshape", xshape_opt[ weightedsel ] );
        out.addMessage( "/gran/"+id+"/yshape", -1 );
        out.addMessage( "/gran/"+id+"/harm", 0.11 );
        out.addMessage( "/gran/"+id+"/mod", 2.1 );
        out.addMessage( "/gran/"+id+"/pan", scale( data.centroid_x[idx], 0, 1, -1, 1 ) );
        out.addMessage( "/gran/"+id+"/amp", scale( data.contour_area[idx], 0., 1., 0.5, 1. ) * amp_mul_opt[ weightedsel ] );
        out.addMessage( "/gran/"+id+"/reverb/sendB", gran_rev_opt[ weightedsel ] );
        
    }
    
    // Filter
    
    ArrayXd filter_amps = lineLookup( data.contour_area, { 0., 0.1, 0.2, 0.4 },
                                                         { 0., 1.,  1.,  0.  } );
    
    out.addMessage("/filter/amp", filter_amps );
    
    if( data.ncontours > 0 )
    {
        out.addMessage("/filter/loop/buffer/idx", 0);
        out.addMessage("/filter/loop/start/ratio", 0.6649);
        out.addMessage("/filter/loop/length/ms", 1952);
        out.addMessage("/filter/loop/transpose", 150);
        out.addMessage("/retrigger/timeramp", fmod( data.elapsed_contour.minCoeff(), 10) );
        out.addMessage("/filter/loop/retrigger/hz", 0);//, #scale(/area, /filter/fade/region[[0]], /filter/fade/region[[3]], 20, 10.));
        out.addMessage("/filter/loop/retrigger/enable", 0);//, #bound(/elapsed/contour) && (/retrigger/timeramp < 0.5));

        vector<int> ids = data.id;
        for_each(ids.begin(), ids.end(), [](int& n){ n++; });

        out.addMessage("/filter/voice",     ids );
        out.addMessage("/filter/Q",         scale( data.contour_area, 0., 1., 0.71, 2.) );
        out.addMessage("/filter/dB",        scale( data.contour_area, 0., 1., -6., 0.));
        out.addMessage("/filter/type",      ArrayXd::Constant( data.ncontours, 2) );
        out.addMessage("/filter/pan",       scale( data.contour_area, 0, 1, -1, 1 ) );

        ArrayXd filter_down_angle = scale( abs( data.defect_rel_mean_angle ), 0., 180, 1, 2.);
        ArrayXd filter_down_area = scale( data.contour_area, 0, 1, 0., 20.);

        out.addMessage("/filter/down/angle",            filter_down_angle );
        out.addMessage("/filter/down/area",             filter_down_area );
        out.addMessage("/filter/downsamp/wobble/time",  ArrayXd::Constant( data.ncontours, 100) );
        out.addMessage("/filter/downsamp/wobble/start", filter_down_angle );
        out.addMessage("/filter/downsamp/wobble/end",   ArrayXd(filter_down_angle + filter_down_area) );
        out.addMessage("/filter/hz",                    mtof( scale( data.contour_area, 0, 1, 120., 60.) ) );
    }
   
    
    // /res/fade/param = nil
    

    return out;
}

/*
 
     /perc/fade/param = quote(/area),
     /perc/fade/region = [ 0, 0, 1, 1],
     /perc/expr = quote( lambda([],
         /perc/drum/trigger = /section/trigger,
         /perc/drum/motor = 0,
         /perc/drum/rate = 1,
         /perc/drum/duration = -1,
         /perc/drum/amp = 1,
         /perc/drum/buffer/index = 12,
         /perc/drum/pan = 0.,
         /perc/drum/xshape = 0,
         /perc/drum/yshape = -5,

         if( bound(/noteOn/incr),
           map(
             lambda([id],
               assign("/perc/clock/"+i+"/trigger", 1 )
             ), /ids[[/noteOn/idx]] + 1
           )
         ),

         if( bound(/area),
           map(
             lambda([i,size, time, x],
               /scaled = clip(size * 10, 0., 1.),
               /time/out = /scaled - clip(time * 0.1, 0., 1),
               assign("/perc/clock/"+i+"/position", [/scaled, /scaled * 100] ),
               assign("/perc/clock/"+i+"/phraseidx", 0 ),
               assign("/perc/clock/"+i+"/scale/index", [0., 14.]),
               assign("/perc/clock/"+i+"/scale/motor", [ (/scaled == 1 ? 20 : 0.001),  500. ]),
               assign("/perc/clock/"+i+"/scale/dur", [1, 100.] ),
               assign("/perc/clock/"+i+"/scale/rate", [1. / (/scaled * 7), /scaled * 10.]),
               assign("/perc/clock/"+i+"/scale/amp", [0., /time/out]),
               assign("/perc/clock/"+i+"/scale/pan", scale(x, 0., 1., -1., 1) ),
               assign("/perc/clock/"+i+"/scale/reverb/sendB", [-60., -6.])
             ), aseq(1, length(/area)), /area, /elapsed/contour, /centroid/x
           ),
           assign("/perc/clock/ * /scale/amp", [0., 0.])
         )
     )),

     /gran/fade/param = quote(/area),
     /gran/fade/region = [0, 0, 1, 1],
     /gran/expr = quote( lambda([],
         /gran/fadein/ms = 20,
         /gran/fadeout/ms = 500, #let ring

         /scale/y = /intervals,
         /scale/x =  aseq( 0, length(/scale/y)-1 ),
         /oct = length(/scale/x),

         /figure/0/y = 0,
         /figure/0/x = 0,

         /figure/1/y = [/oct+1, -/oct, 0],
         if( /section/trigger || !bound(/prev/figure/1/x), # repeat the same phrase in the section
           progn(
             /randset = sort( /~/nrand( length(/figure/1/y) ) ),
             /figure/1/x = int32(scale(/randset, min(/randset), max(/randset), 0, 8))
           ),
           /figure/1/x = /prev/figure/1/x
         ),

         /figure/2/y = [0, 1,  2],
         /figure/2/x = [0, 4,  5],

         /figure/3/y = [0, 1,  2, 3, 4, 5],
         /figure/3/x = [0, 4,  5, 8, 9, 10],

         /steps     = [0,  1,   2,  3   ], #offset for figure
         
         # weighted random selections
         /weights      = [0.5,  0.01,  0.9,  0,   0,   0],
         /octave/opt   = [6,    -2,    6,   6,   6,   7] * 12 ,
         /dur/env/opt  = [1,    500,   0.5,  200, 200, 300],
         /dur/beat/opt = [10,   1000,  100,  50,  200, 100],
         /fig/opt      = [0,    0,     3,    2,   1,   0],
         /amp/mul/opt  = [1,    1,     1,    1,   1,   1],
         /xshape/opt   = [-5,   -10,   -5,    -5,   -5,  -5],
         /gran/rev/opt = [0,    0.05,  0,    0,    0,   0],

         if( bound(/noteOn/incr),
           map(
             lambda([id, incr, idx],
               /weightedsel = /~/weightedrand( /weights ),
               assign( "/gran/"+id+"/scale/x", /scale/x ),
               assign( "/gran/"+id+"/scale/y", /scale/y ),
               assign( "/gran/"+id+"/baseNote", /tonic + /octave/opt[[ /weightedsel ]] ),
               assign( "/gran/"+id+"/step",  /steps[[ incr ]] ),
               assign( "/gran/"+id+"/fig/x", value("/figure/" +/fig/opt[[ /weightedsel ]]+ "/x") ),
               assign( "/gran/"+id+"/fig/y", value("/figure/" +/fig/opt[[ /weightedsel ]]+ "/y") ),
               assign( "/gran/"+id+"/dur", /dur/env/opt[[ /weightedsel ]] ),
               assign( "/gran/"+id+"/phrase/dur", length( value("/gran/"+id+"/fig/x") ) * /dur/beat/opt[[ /weightedsel ]] ),
               assign( "/gran/"+id+"/xshape", /xshape/opt[[ /weightedsel ]] ),
               assign( "/gran/"+id+"/yshape", -1 ),
               assign( "/gran/"+id+"/harm", 0.11 ),
               assign( "/gran/"+id+"/mod", 2.1 ),
               assign( "/gran/"+id+"/pan", scale( /centroid/x[[ idx ]], 0, 1, -1, 1 ) ),
               assign( "/gran/"+id+"/amp", scale( /area[[/idx]], 0., 1., 0.5, 1. ) * /amp/mul/opt[[ /weightedsel ]] ),
               assign( "/gran/"+id+"/reverb/sendB", /gran/rev/opt[[ /weightedsel ]] )

             ), (/ids[[/noteOn/idx]] + 1), ( /noteOn/incr % length(/steps) ), /noteOn/idx
           )
         )
     )),
     /filter/fade/param = quote(/area),
     /filter/fade/region = [0.0, 0.1, 0.2, 0.4],
     /filter/expr = quote(lambda([],
         /filter/loop/buffer/idx = 0,
         /filter/loop/start/ratio = 0.6649,
         /filter/loop/length/ms = 1952,
         /filter/loop/transpose = 150,
         /retrigger/timeramp = min(/elapsed/contour) % 10,
         /filter/loop/retrigger/hz = 0, #scale(/area, /filter/fade/region[[0]], /filter/fade/region[[3]], 20, 10.),
         /filter/loop/retrigger/enable = 0, #bound(/elapsed/contour) && (/retrigger/timeramp < 0.5),
         /filter/voice = /ids + 1,
         /filter/Q = scale(/area, /filter/fade/region[[0]], /filter/fade/region[[3]], 0.71, 2.),
         /filter/dB = scale(/area, /filter/fade/region[[0]], /filter/fade/region[[3]], -6., 0.),
         /filter/type = nfill(/count, 2),
         /filter/pan = scale(/centroid/x, 0, 1, -1, 1),
         /filter/down/angle = scale( abs(/defect/rel/mean/angle), 0., 180, 1, 2.),
         /filter/down/area = scale(/area, /filter/fade/region[[0]], /filter/fade/region[[3]], 0., 20.),
         /filter/downsamp/wobble/time = nfill(/count, 100),
         /filter/downsamp/wobble/start = /filter/down/angle,
         /filter/downsamp/wobble/end = /filter/down/angle + /filter/down/area,
         /filter/hz = mtof( scale(/area, /filter/fade/region[[0]], /filter/fade/region[[3]], 120., 60.) )
     )),
     /res/fade/param = nil
 ))
 */

