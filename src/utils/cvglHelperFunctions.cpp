

#include "cvglHelperFunctions.hpp"
#include <iostream>

using namespace std;

double cvgl::ntom( const string & note )
{
    const vector<string> names_lower = {"c", "c#", "db", "d", "d#", "eb", "e", "f", "f#", "gb", "g", "g#", "ab", "a", "a#", "bb", "b" };
    const vector<int32_t> names_step = {0, 1, 1, 2, 3, 3, 4, 5, 6, 6, 7, 8, 8, 9, 10, 10, 11};
    
    int32_t oct = 0;
    double cent_offset = 0;
    string note_name(note);
    
    const size_t found = note.find(":");
    if( found != std::string::npos )
    {
        note_name = note.substr(0, found); // found pos is also the length of the preceeding string
        const string offset_str = note.substr(found+1);
        
        const size_t minus_pos = offset_str.find("-");

        if( minus_pos != string::npos )
        {
            oct = stoi( offset_str.substr(0, minus_pos) ); // found pos is also the length of the preceeding string
            cent_offset = stod( offset_str.substr(minus_pos) );
        }
        else
        {
            const size_t plus_pos = offset_str.find("+");
            if( plus_pos != string::npos )
            {
                oct = stoi( offset_str.substr(0, plus_pos) ); // found pos is also the length of the preceeding string
                cent_offset = stod( offset_str.substr(plus_pos) );
            }
            else
            {
                oct = stoi( offset_str );
            }
        }
        
    }
    
    note_name[0] = tolower(note_name[0]);
    auto it = find( names_lower.begin(), names_lower.end(), note_name);
    if( it != names_lower.end() )
    {
        return ( names_step[ it - names_lower.begin() ] + (12*(oct+1)) + (cent_offset * 0.01) );
    }
    else
    {
        // didn't find!!
        return 0;
    }
   
}


double cvgl::lineLookup( double t, std::vector<double> x, std::vector<double> y )
{
    if( t <= x[0] )
        return y[0];
    else if( t == x.back() )
         return y.back();
    
    size_t len = x.size();
    for( int i = 1; i < len; ++i )
    {
        int prevIdx = i - 1;
        if( t >= x[prevIdx] && t < x[i] )
        {
            double x_range = x[i] - x[prevIdx];
            double y_range = y[i] - y[prevIdx];
            double rel_t = (t - x[prevIdx]) / x_range;
            return y[prevIdx] + (rel_t * y_range);
        }
    }
    
    
//    if( i >= len-1 )
    
    return y.back();
   
}

Eigen::ArrayXd cvgl::lineLookup( const Eigen::ArrayXd& t, const std::vector<double>& x, const std::vector<double>& y )
{
    Eigen::ArrayXd amps = t.unaryExpr([&]( double _t ){
        return lineLookup(_t, x, y);
    });
    return std::move(amps);
}


