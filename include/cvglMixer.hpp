#pragma once

#include <unordered_map>
#include <string>
#include <Eigen/Dense>
#include "MapOSC.hpp"

using namespace Eigen;
using namespace std;

struct cvglMixer
{
    
    int nsliders = 9;
    ArrayXd pregain, gain, amp;
    unordered_map<std::string, int> chan;
    
    double m_min_dB = -60;
    double m_max_dB = 20;
    double m_dB_range = m_max_dB + abs(m_min_dB);
    
    double m_min_amp_thresh = dbtoa(m_min_dB);
    
    cvglMixer()
    {
        pregain = ArrayXd::Constant(nsliders, -100);
        gain = ArrayXd::Constant(nsliders, -100);
        amp = ArrayXd::Zero(nsliders);


        chan.emplace("/perc",       0); // zero indexed
        chan.emplace("/gran",       1);
        chan.emplace("/res",        2);
        chan.emplace("/filter",     3);

        chan.emplace("/main",       8);
        
    }
    
    void addPregainMidi( MapOSC & b )
    {
        b.addMessage("/mixer/knob/1", getPregainMidi("/perc") );
        b.addMessage("/mixer/knob/2", getPregainMidi("/gran") );
        b.addMessage("/mixer/knob/3", getPregainMidi("/res") );
        b.addMessage("/mixer/knob/4", getPregainMidi("/filter") );
    }
    
    void applyAddGains( MapOSC & b )
    {
        applyPregain();
        
        b.addMessage("/perc/amp_scale", getAmp("/perc") );
        b.addMessage("/gran/amp_scale", getAmp("/gran") );
        b.addMessage("/res/amp_scale", getAmp("/res") );
        b.addMessage("/filter/amp_scale", getAmp("/filter") );
        b.addMessage("/main_dB", getGain("/main") );
    }
    
    void proc( MapOSC & b)
    {
            
        for( auto& m : b.getMap() )
        {
            const std::string& addr = m.first;
            const auto& atomVec = m.second;

            if( addr == "/mixer/fader/1") {
                midi_gain(1, atomVec.getInt() );
            } else if( addr == "/mixer/fader/2") {
                midi_gain(2, atomVec.getInt() );
            } else if( addr == "/mixer/fader/3") {
                midi_gain(3, atomVec.getInt() );
            } else if( addr == "/mixer/fader/4") {
                midi_gain(4, atomVec.getInt() );
            } else if( addr == "/mixer/fader/5") {
                midi_gain(5, atomVec.getInt() );
            } else if( addr == "/mixer/fader/6") {
                midi_gain(6, atomVec.getInt() );
            } else if( addr == "/mixer/fader/7") {
                midi_gain(7, atomVec.getInt() );
            } else if( addr == "/mixer/fader/8") {
                midi_gain(8, atomVec.getInt() );
            } else if( addr == "/mixer/fader/main") {
                midi_gain(9, atomVec.getInt() );
            } else if( addr == "/mixer/knob/1") {
                midi_pregain(1, atomVec.getInt() );
            } else if( addr == "/mixer/knob/2") {
                midi_pregain(2, atomVec.getInt() );
            } else if( addr == "/mixer/knob/3") {
                midi_pregain(3, atomVec.getInt() );
            } else if( addr == "/mixer/knob/4") {
                midi_pregain(4, atomVec.getInt() );
            } else if( addr == "/mixer/knob/5") {
                midi_pregain(5, atomVec.getInt() );
            } else if( addr == "/mixer/knob/6") {
                midi_pregain(6, atomVec.getInt() );
            } else if( addr == "/mixer/knob/7") {
                midi_pregain(7, atomVec.getInt() );
            } else if( addr == "/mixer/knob/8") {
                midi_pregain(8, atomVec.getInt() );
            }
        }
        
    }
    
    // range: -60 to 20 dB
    inline double midi2dB( int midi ){ return ((midi / 127.) * 80.) - 60.; }
    inline int32_t dB2midi( double db ){ return int32_t( ((db + 60.) / 80.) * 127.); }
    
    inline double dbtoa(double db){ return pow(10., (db / 20.)); }
    inline ArrayXd dbtoa(ArrayXd db){ return pow(10., (db / 20.)); }
    
    
    // setters
    inline void midi_gain(int channel, int midi_val)
    {
        if( channel > 0 && channel <= nsliders ) {
            gain(channel-1) = midi2dB( midi_val );
            //    std::cout << gain << std::endl;
        }
    }
    
    inline void midi_pregain(int channel, int midi_val)
    {
        if( channel > 0 && channel < nsliders ){ // no pregain on the main
            pregain[channel-1] = midi2dB( midi_val );
        }
    }
    
    inline void set_pregain(const string & channel, double db)
    {
        if( chan.count(channel) ){
            pregain[ chan[channel] ] = db;
        }
    }
    
    
    // calc
    void applyPregain()
    {
        amp = dbtoa(gain + pregain).unaryExpr([&](double x){return (x > m_min_amp_thresh ? x : 0.); });

        //     std::cout << amp << std::endl;
    }
    
    //getters
    
    inline double getAmp(const string & channel)
    {
        if( chan.count(channel) ){
            //    std::cout << amp( chan[channel] ) << std::endl;
            return amp( chan[channel] );
        }
        return 0;
    }
    
    inline double getGain(const string & channel)
    {
        if( chan.count(channel) ){
            //    std::cout << amp( chan[channel] ) << std::endl;
            return gain( chan[channel] );
        }
        return 0;
    }
    
    inline double getPregain(const string & channel)
    {
        if( chan.count(channel) ){
            return pregain( chan[channel] );
        }
        return 0;
    }
    
    inline int32_t getPregainMidi(const string & channel)
    {
        if( chan.count(channel) ){
            int32_t midi = dB2midi(pregain( chan[channel] ));
            return (midi < 0 ? 0 : (midi > 127 ? 127 : midi));
        }
        return 0;
    }
    
    
    
    void initMidi( MapOSC & b )
    {
        
        int32_t zeroMidi_dB = dB2midi(0);

        b.addMessage("/mixer/knob/1", zeroMidi_dB );
        b.addMessage("/mixer/knob/2", zeroMidi_dB );
        b.addMessage("/mixer/knob/3", zeroMidi_dB );
        b.addMessage("/mixer/knob/4", zeroMidi_dB );
        b.addMessage("/mixer/knob/5", zeroMidi_dB );
        b.addMessage("/mixer/knob/6", zeroMidi_dB );
        b.addMessage("/mixer/knob/7", zeroMidi_dB );
        b.addMessage("/mixer/knob/8", zeroMidi_dB );

        
        b.addMessage("/mixer/fader/1", zeroMidi_dB );
        b.addMessage("/mixer/fader/2", zeroMidi_dB  );
        b.addMessage("/mixer/fader/3", zeroMidi_dB );
        b.addMessage("/mixer/fader/4", zeroMidi_dB );
        b.addMessage("/mixer/fader/5", zeroMidi_dB );
        b.addMessage("/mixer/fader/6", zeroMidi_dB  );
        b.addMessage("/mixer/fader/7", zeroMidi_dB  );
        b.addMessage("/mixer/fader/8", zeroMidi_dB  );
        b.addMessage("/mixer/fader/main", 0 );
        
        midi_gain(0, zeroMidi_dB );
        midi_gain(1, zeroMidi_dB );
        midi_gain(2, zeroMidi_dB );
        midi_gain(3, zeroMidi_dB );
        midi_gain(4, zeroMidi_dB );
        midi_gain(5, zeroMidi_dB );
        midi_gain(6, zeroMidi_dB );
        midi_gain(7, zeroMidi_dB );
    }
    
    
    
};
