#pragma once

#include <vector>
#include <Eigen/Dense>
#include <string>

namespace cvgl {

inline double clip(double v, double min, double max)
{
    return (v < min ? min : (v > max ? max : v));
}

inline Eigen::ArrayXd clip(const Eigen::ArrayXd& v, double min, double max)
{
    return v.unaryExpr([&](double x){return clip(x, min, max); });
}

inline double wrap(double x, double x_min, double x_max)
{
    return fmod( fmod((x - x_min), (x_max - x_min)) + (x_max - x_min), (x_max - x_min)) + x_min;
}

inline double scale(double v, double in_min, double in_max, double out_min, double out_max)
{
    const double in_range = in_max - in_min;
    return (( (v - in_min) / (in_range == 0 ? 1 : in_range) ) * (out_max - out_min)) + out_min;
}

inline Eigen::ArrayXd scale(const Eigen::ArrayXd& v, double in_min, double in_max, double out_min, double out_max)
{
    return v.unaryExpr([&](double x){ return scale(x, in_min, in_max, out_min, out_max); });
}

template <typename T>
inline std::vector<T> scale(const std::vector<T>& v, const T in_min, const T in_max, const T out_min, const T out_max)
{
    T in_range = in_max - in_min;
    T out_range = out_max - out_min;
    std::vector<T> ret( v.size() );
    for( auto& q : v )
    {
        ret.emplace_back( ( ((q - in_min) / in_range) * out_range) + out_min );
    }
    return ret;
}

inline double scale_clip(double v, double in_min, double in_max, double out_min, double out_max)
{
    double clip_min = out_min > out_max ? out_max : out_min;
    double clip_max = out_max < out_min ? out_min : out_max;

    return clip( scale( v, in_min, in_max, out_min, out_max), clip_min, clip_max);
}

inline Eigen::ArrayXd scale_clip(const Eigen::ArrayXd& v, double in_min, double in_max, double out_min, double out_max)
{
    double clip_min = out_min > out_max ? out_max : out_min;
    double clip_max = out_max < out_min ? out_min : out_max;

    return clip( scale( v, in_min, in_max, out_min, out_max), clip_min, clip_max);
}


template <typename T>
inline std::vector<T> scale_clip(const std::vector<T>& v, const T in_min, const T in_max, const T out_min, const T out_max)
{
    double clip_min = out_min > out_max ? out_max : out_min;
    double clip_max = out_max < out_min ? out_min : out_max;

    T in_range = in_max - in_min;
    T out_range = out_max - out_min;
    std::vector<T> ret( v.size() );
    for( auto& q : v )
    {
        T val = ( ((q - in_min) / in_range) * out_range) + out_min;
        val = (val < clip_min ? clip_min : (val > clip_max ? clip_max : val));
        ret.emplace_back(val);
    }
    return ret;
}



inline double sum( std::vector<double> &vec )
{
    double _sum = 0;
    for (auto& n : vec){
        _sum += n;
    }
    return _sum;
}


inline int32_t round(double x)
{
    return int32_t(x + 0.5);
}

inline std::vector<double> dur2x( std::vector<double> & vec)
{
    std::vector<double> seq_x{0};
    for( size_t i = 0; i < vec.size()-1; ++i)
    {
        seq_x.emplace_back( seq_x.back() + vec[i] );
    }
    return seq_x;
}

inline Eigen::ArrayXd dur2x( Eigen::ArrayXd & vec)
{
    Eigen::ArrayXd seq_x( vec.size()+1 );
    seq_x(0) = 0;
    
    for( size_t i = 1; i < vec.size()+1; i++)
    {
        seq_x(i) = seq_x(i-1) + vec[i-1];
    }
    
    return seq_x;
}

inline Eigen::ArrayXd mtof( const Eigen::ArrayXd & v, double a4 = 440.)
{
    return a4 * pow(2., (v - 69.) / 12.) ;
}

inline double mtof( double v, double a4 = 440.)
{
    return a4 * pow(2., (v - 69.) / 12.) ;
}

inline double ftom( double v, double a4 = 440. )
{
    return 69.0 + (12.0 * log2( v / a4 ));
}

/*
inline Eigen::ArrayXd ftom( const Eigen::ArrayXd & v, double a4 = 440. )
{
    return 69 + (12 * log2( v / a4 ));
}*/


inline double erb( double center_hz, double ratio)
{
    const double half_bandwidth = (24.7 * ((0.00437 * center_hz) + 1)) * 0.5;
    return center_hz + (half_bandwidth * ratio);
}

inline double tanhScale( double x, double in_min, double in_max, double curveScalar = 1 )
{
    const double curveTanh = tanh(curveScalar);
    const double scaled_x = scale( x, in_min, in_max, -1., 1.);
    return scale( tanh( scaled_x * curveTanh ), -curveTanh, curveTanh, 0., 1. );
}

inline double scaleInterval( long step, std::vector<double> scale )
{
    if( step < 0 ) {
        return scale[ long( step % scale.size() ) ] + ( long( ((step+1) / (long)scale.size() ) - 1 ) * 12 );
    }
    else
        return scale[ long( step % scale.size() ) ] + long( (step / scale.size() ) * 12 );
}


inline double dbtoa(double db){ return pow(10., (db / 20.)); }


double ntom( const std::string & note );

double lineLookup( double t, std::vector<double> x, std::vector<double> y );

Eigen::ArrayXd lineLookup( const Eigen::ArrayXd& t, const std::vector<double>& x, const std::vector<double>& y );

template <typename T>
std::vector<T> aseq(T from, T to, T step = 1)
{
    std::vector<T> ret;
    T incr = from;
    while( incr <= to )
    {
        ret.emplace_back(incr);
        incr += step;
    }
    
    return ret;
}

inline double easeInOutQuad(double x)
{
    return x < 0.5 ? 2 * x * x : 1 - pow(-2 * x + 2, 2) / 2;
}

inline double easeInOutSine(double x)
{
    return -(cos(M_PI * x) - 1) * 0.5;
}

inline double easeInSine(double x)
{
     return 1 - cos((x * M_PI) * 0.5);
}

inline double easeOutSine(double x)
{
     return sin((x * M_PI) * 0.5);
}

inline double easeInExpo(double x)
{
    return x == 0 ? 0 : pow(2, 10 * x - 10);
}

inline double easeOutExpo(double x)
{
    return x == 1 ? 1 : 1 - pow(2, -10 * x);

}

}
