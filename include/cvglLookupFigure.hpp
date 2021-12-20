#pragma once

#include <vector>

using namespace std;

struct LookupFigure
{
    vector<double> x;
    vector<double> y;
    
    LookupFigure(){}

    LookupFigure( const vector<double> & _x, const vector<double> & _y) : x(_x), y(_y) {}
    
    LookupFigure( vector<double> _y) : y(_y)
    {
        makeIdx();
    }

    inline void makeIdx()
    {
        x.clear();
        x.reserve(y.size());
        for( int i = 0; i < y.size(); ++i)
            x.emplace_back(i);
    }
    
    
};
