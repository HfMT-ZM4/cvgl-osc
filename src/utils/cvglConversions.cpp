#include "cvglConversions.hpp"

#include <deque>
#include <iostream>

using namespace std;
using namespace cv;

namespace cvgl
{
    
    std::vector<float> getRGBA( OSCAtomVector & msg )
    {
        std::vector<float> _rgba;
        for( const auto& a : msg.getAtomVector() )
        {
            _rgba.emplace_back( a->getFloat() );
        }
        
        for( int i = (int)_rgba.size()-1; i < 3; i++)
        {
            _rgba.emplace_back(1);
        }
        
        return _rgba;
    }
    
    template<typename T>
    void pointMatToVertex(const Mat& points, cvglObject& vertexObj, const float halfScreenW, const float halfScreenH )
    {
        vertexObj.newObj();
        for(int i = 0; i < points.rows; i++)
        {
            const T* Mi = points.ptr<T>(i);
            for(int j = 0; j < points.cols; j++)
            {
                const T& Mij = Mi[j];
                vertexObj.addVertex( cvglVertex({
                    { static_cast<float>((Mij.x - halfScreenW) / halfScreenW),
                        static_cast<float>(-(Mij.y - halfScreenH) / halfScreenH), 0}
                }));
            }
        }
        vertexObj.endObj();
    }
    
    void pointMatToVertex(const Mat& points, cvglObject& vertexObj, const float halfScreenW, const float halfScreenH )
    {
        switch( points.depth() )
        {
            case CV_32S:
                pointMatToVertex<Point2i>(points, vertexObj, halfScreenW, halfScreenH );
                break;
            case CV_32F:
                pointMatToVertex<Point2f>(points, vertexObj, halfScreenW, halfScreenH );
                break;
            case CV_64F:
                pointMatToVertex<Point2d>(points, vertexObj, halfScreenW, halfScreenH );
                break;
            default:
                cout << __func__ << "unknown type, maybe long? "  << endl;
                break;
        }
    }
    
    template<typename T>
    void pointMatToVertex(const Mat& points, unique_ptr<cvglObject>& vertexObj, const float halfScreenW, const float halfScreenH )
    {
        vertexObj->newObj();
        for(int i = 0; i < points.rows; i++)
        {
            const T* Mi = points.ptr<T>(i);
            for(int j = 0; j < points.cols; j++)
            {
                const T& Mij = Mi[j];
                vertexObj->addVertex( cvglVertex({
                    static_cast<float>((Mij.x - halfScreenW) / halfScreenW),
                    static_cast<float>(-(Mij.y - halfScreenH) / halfScreenH)
                }));
            }
        }
        vertexObj->endObj();
    }
    
    void pointMatToVertex(const Mat& points, unique_ptr<cvglObject>& vertexObj, const float halfScreenW, const float halfScreenH )
    {
        switch( points.depth() )
        {
            case CV_32S:
                pointMatToVertex<Point2i>(points, vertexObj, halfScreenW, halfScreenH );
                break;
            case CV_32F:
                pointMatToVertex<Point2f>(points, vertexObj, halfScreenW, halfScreenH );
                break;
            case CV_64F:
                pointMatToVertex<Point2d>(points, vertexObj, halfScreenW, halfScreenH );
                break;
            default:
                cout << __func__ << "unknown type, maybe long? " << points.depth() << endl;
                break;
        }
    }
    
    cv::Point2f normalized( cv::Point2f pt )
    {
        return pt / sqrt(pt.x*pt.x + pt.y*pt.y );
    }
    
    cv::Point2f normalize( cv::Point2f pt )
    {
        return pt / sqrt(pt.x*pt.x + pt.y*pt.y );
    }
    
    cv::Point2f direction( cv::Point2f a, cv::Point2f b )
    {
        return normalized( a - b );
    }
    
    cv::Point2f normal( cv::Point2f pt )
    {
        return cv::Point2f(-pt.y, pt.x);
    }
    
    
    
    std::array<cv::Point2f,2> getOffsetLinePoints( cv::Point2f p0, cv::Point2f p1, cv::Point2f p2, float thickness )
    {
        // 1) define the line between the two points
        Point2f line = p2 - p1;
        
        // 2) find the normal vector of this line
        Point2f normal = normalized( Point2f(-line.y, line.x) );
        
        // 3) find the tangent vector at both the end points:
        //        -if there are no segments before or after this one, use the line itself
        //        -otherwise, add the two normalized lines and average them by normalizing again
        Point2f tangent1 = (p0 == p1) ? line : normalized( normalized(p1-p0) + line );
        
        if( (p0 == p1) )
            cout << " prob not " << (p0 == p1) << endl;
//        Point2f tangent2 = (p2 == p3) ? line : normalized( normalized(p3-p2) + line );

//        Point2f a = p1 - (normal * (thickness * 0.5));
//        Point2f b = p1 + (normal * (thickness * 0.5));
//
        // 4) find the miter line, which is the normal of the tangent
        Point2f miter1 = Point2f(-tangent1.y, tangent1.x);
        // cout << "tangent " << tangent1 << " m " << miter1 << endl;
//        Point2f miter2 = Point2f(-tangent2.y, tangent2.x);
        
        // find length of miter by projecting the miter onto the normal,
        // take the length of the projection, invert it and multiply it by the thickness:
        //        length = thickness * ( 1 / |normal|.|miter| )
        float length1 = thickness / miter1.dot(normal);
//        float length2 = thickness / normal.dot(miter2);
        
        Point2f a =  p1 - length1 * miter1;
        Point2f b =  p1 + length1 * miter1;
        return std::array<cv::Point2f,2>({a,b});
        
    }
    
    void pointMatToPolygonLineVertex(const Mat& points, unique_ptr<cvglObject>& vertexObj, const float halfScreenW, const float halfScreenH, const float thickness, const bool closed )
    {
        const float halfThickness = thickness * 0.5;
      //  std::vector<GLuint> index_array;
        
        std::deque<Point2f> lineTurn, firstSegment;
        
        Point2f p0, p1, p2;
        int maxi = points.rows-1;
        int maxj = points.cols-1;
        
        vertexObj->newObj();
        for(int i = 0; i < points.rows; i++)
        {
            const Point2f* Mi = points.ptr<Point2f>(i);
            for(int j = 0; j < points.cols; j++)
            {
                const Point2f& Mij = Mi[j];

                lineTurn.emplace_back(Mij);

                if( lineTurn.size() == 3 )
                {
                    if( firstSegment.size() == 0 )
                        firstSegment = lineTurn;
                
                    p0 = lineTurn[0];
                    p1 = lineTurn[1];
                    p2 = lineTurn[2];
                    
                    auto adj_pts = getOffsetLinePoints(p0, p1, p2, halfThickness);
                    
                    vertexObj->addVertex( cvglVertex({
                        static_cast<float>((adj_pts[0].x - halfScreenW) / halfScreenW),
                        static_cast<float>(-(adj_pts[0].y - halfScreenH) / halfScreenH)
                    }));
                    vertexObj->addVertex( cvglVertex({
                        static_cast<float>((adj_pts[1].x - halfScreenW) / halfScreenW),
                        static_cast<float>(-(adj_pts[1].y - halfScreenH) / halfScreenH)
                    }));
                    
                    
                    if( i == maxi && j == maxj && closed)
                    {
                        // last point, add thickness to last point, or do something if closed loop
                        adj_pts = getOffsetLinePoints(p1, p2, firstSegment[0], halfThickness);

                        vertexObj->addVertex( cvglVertex({
                            static_cast<float>((adj_pts[0].x - halfScreenW) / halfScreenW),
                            static_cast<float>(-(adj_pts[0].y - halfScreenH) / halfScreenH)
                        }));
                        vertexObj->addVertex( cvglVertex({
                            static_cast<float>((adj_pts[1].x - halfScreenW) / halfScreenW),
                            static_cast<float>(-(adj_pts[1].y - halfScreenH) / halfScreenH)
                        }));
                    
                        // last point, add thickness to last point, or do something if closed loop
                        adj_pts = getOffsetLinePoints(p2, firstSegment[0], firstSegment[1], halfThickness);
                      
                        vertexObj->addVertex( cvglVertex({
                            static_cast<float>((adj_pts[0].x - halfScreenW) / halfScreenW),
                            static_cast<float>(-(adj_pts[0].y - halfScreenH) / halfScreenH)
                        }));

                        vertexObj->addVertex( cvglVertex({
                            static_cast<float>((adj_pts[1].x - halfScreenW) / halfScreenW),
                            static_cast<float>(-(adj_pts[1].y - halfScreenH) / halfScreenH)
                        }));
                        
                        // last point, add thickness to last point, or do something if closed loop
                        adj_pts = getOffsetLinePoints(firstSegment[0], firstSegment[1], firstSegment[2], halfThickness);
                        
                        vertexObj->addVertex( cvglVertex({
                            static_cast<float>((adj_pts[0].x - halfScreenW) / halfScreenW),
                            static_cast<float>(-(adj_pts[0].y - halfScreenH) / halfScreenH)
                        }));
                        
                        vertexObj->addVertex( cvglVertex({
                            static_cast<float>((adj_pts[1].x - halfScreenW) / halfScreenW),
                            static_cast<float>(-(adj_pts[1].y - halfScreenH) / halfScreenH)
                        }));
                    
                    }

                    lineTurn.pop_front();

                }
               
               
            }
        }
        vertexObj->endObj();
    }
    
    void pointsToPolygonLineVertex(const vector<Point2f>& points, unique_ptr<cvglObject>& vertexObj, const float halfScreenW, const float halfScreenH, const float thickness, const bool closed )
    {
        const float halfThickness = thickness * 0.5;
        //  std::vector<GLuint> index_array;
        cout << "pointsToPolygonLineVertex" << endl;
        std::deque<Point2f> lineTurn, firstSegment;
        
        Point2f p0, p1, p2;
        int npoints = (int)points.size();
        int maxIdx = npoints-1;
        
        vertexObj->newObj();
        for(int i = 0; i < npoints; i++)
        {
            lineTurn.emplace_back(points[i]);
            
            if( lineTurn.size() == 3 )
            {
                if( firstSegment.size() == 0 )
                    firstSegment = lineTurn;
                
                p0 = lineTurn[0];
                p1 = lineTurn[1];
                p2 = lineTurn[2];
                
                if(p1 != p2)
                {
                    
                    auto adj_pts = getOffsetLinePoints(p0, p1, p2, halfThickness);
                    
                    cout << adj_pts[0] << " " << adj_pts[1] << endl;
                    
                    vertexObj->addVertex( cvglVertex({
                        static_cast<float>((adj_pts[0].x - halfScreenW) / halfScreenW),
                        static_cast<float>(-(adj_pts[0].y - halfScreenH) / halfScreenH)
                    }));
                    vertexObj->addVertex( cvglVertex({
                        static_cast<float>((adj_pts[1].x - halfScreenW) / halfScreenW),
                        static_cast<float>(-(adj_pts[1].y - halfScreenH) / halfScreenH)
                    }));
                    
                    
                    if( i == maxIdx && closed)
                    {
                        // last point, add thickness to last point, or do something if closed loop
                        adj_pts = getOffsetLinePoints(p1, p2, firstSegment[0], halfThickness);
                        cout << adj_pts[0] << " " << adj_pts[1] << endl;
                        
                        vertexObj->addVertex( cvglVertex({
                            static_cast<float>((adj_pts[0].x - halfScreenW) / halfScreenW),
                            static_cast<float>(-(adj_pts[0].y - halfScreenH) / halfScreenH)
                        }));
                        vertexObj->addVertex( cvglVertex({
                            static_cast<float>((adj_pts[1].x - halfScreenW) / halfScreenW),
                            static_cast<float>(-(adj_pts[1].y - halfScreenH) / halfScreenH)
                        }));
                        
                        // last point, add thickness to last point, or do something if closed loop
                        adj_pts = getOffsetLinePoints(p2, firstSegment[0], firstSegment[1], halfThickness);
                        cout << adj_pts[0] << " " << adj_pts[1] << endl;
                        
                        vertexObj->addVertex( cvglVertex({
                            static_cast<float>((adj_pts[0].x - halfScreenW) / halfScreenW),
                            static_cast<float>(-(adj_pts[0].y - halfScreenH) / halfScreenH)
                        }));
                        
                        vertexObj->addVertex( cvglVertex({
                            static_cast<float>((adj_pts[1].x - halfScreenW) / halfScreenW),
                            static_cast<float>(-(adj_pts[1].y - halfScreenH) / halfScreenH)
                        }));
                        
                        // last point, add thickness to last point, or do something if closed loop
                        adj_pts = getOffsetLinePoints(firstSegment[0], firstSegment[1], firstSegment[2], halfThickness);
                        cout << adj_pts[0] << " " << adj_pts[1] << endl;
                        
                        vertexObj->addVertex( cvglVertex({
                            static_cast<float>((adj_pts[0].x - halfScreenW) / halfScreenW),
                            static_cast<float>(-(adj_pts[0].y - halfScreenH) / halfScreenH)
                        }));
                        
                        vertexObj->addVertex( cvglVertex({
                            static_cast<float>((adj_pts[1].x - halfScreenW) / halfScreenW),
                            static_cast<float>(-(adj_pts[1].y - halfScreenH) / halfScreenH)
                        }));
                        
                    }
                }
                
                
                lineTurn.pop_front();
            }
              
        }
        vertexObj->endObj();
    }
    
    pair<Point2f, float> computeMiter(cv::Point2f lineA, cv::Point2f lineB, float halfThickness)
    {
//        cout << "computeMiter" << lineA << " " << lineB << endl;
        auto tangent = normalized( lineA + lineB );
        auto miter = normal(tangent);
        auto tmp = normal(lineA);
        
        auto miterLen = halfThickness / miter.dot(tmp);
        
  //      cout << "results " << tangent << " " << miter << " " << tmp << " " << miterLen << "\n" << endl;

        return make_pair(miter, miterLen);
    }
    
    vector< pair<Point2f, float> > getNormals( vector<Point2f> points, const bool closed )
    {
        vector< pair<Point2f, float> > normals;
        const auto nullPt = Point2f(-111,-111);
        auto curNormal = nullPt;
        
        if( closed )
            points.emplace_back( points.front() );
        
        int npoints = (int)points.size();

        for(int i = 1; i < npoints; i++)
        {
            auto last = points[i-1];
            auto cur = points[i];
            auto next = i < npoints-1 ? points[i+1] : nullPt;
            
            auto lineA = direction(cur, last);
            
            if( curNormal == nullPt )
                curNormal = normal(lineA);
        
            if( i == 1 )
                normals.emplace_back( curNormal, 1 );

            if( next == nullPt )
            {
                curNormal = normal(lineA); // pretty sure this isn't necessary
                normals.emplace_back( curNormal, 1 );
            }
            else
            {
                if( next == cur )
                {
                    curNormal = normal(lineA); // pretty sure this isn't necessary
                    normals.emplace_back( curNormal, 1.0 );
                }
                else
                {
                    auto lineB = direction(next, cur);
                    normals.emplace_back( computeMiter(lineA, lineB, 1.0) );
                }
                
                
            }
            
        }
        
        if( npoints > 2 && closed )
        {
            auto last2 = points[npoints-2];
            auto cur2 = points[0];
            auto next2 = points[1];
            
            auto lineA = direction(cur2, last2);

            auto lineB = direction(next2, cur2);
            curNormal = normal(lineA);

//            cout << "lineB" << next2 << " " << cur2 << endl;

            auto miterLen2 = computeMiter(lineA, lineB, 1.0);
            normals[0] = miterLen2;
            /*
            normals.back() = miterLen2;
            normals.pop_back(); // ?
             */
        }
        
        return normals;
    }
    
    void linePointsToPolygon(const vector<Point2f>& points, unique_ptr<cvglObject>& vertexObj, const float halfScreenW, const float halfScreenH, const float thickness, const bool closed )
    {
        const float halfThickness = thickness * 0.5;

        auto normalMiters = getNormals(points, closed);
        
    //    cout << normalMiters.size() << " " << points.size() << endl;

        int npoints = (int)points.size();
        
        vertexObj->newObj();
        for(int i = 0; i < npoints; i++)
        {
            auto normLen = normalMiters[i];
            auto norm = normLen.first;
            auto len = normLen.second;
       //     cout << "norm >> " << norm << " " << len << endl;

            auto refPt = points[i];
            
            auto p1 = refPt + norm * len * halfThickness;
            auto p2 = refPt + norm * -len * halfThickness;

//            cout << "points " << p1 << " " << p2 << endl;
            vertexObj->addVertex( cvglVertex({
                static_cast<float>((p1.x - halfScreenW) / halfScreenW),
                static_cast<float>(-(p1.y - halfScreenH) / halfScreenH)
            }));
            
            vertexObj->addVertex( cvglVertex({
                static_cast<float>((p2.x - halfScreenW) / halfScreenW),
                static_cast<float>(-(p2.y - halfScreenH) / halfScreenH)
            }));
          
        }
        
        if( closed )
        {
            auto normLen = normalMiters.front();
            auto norm = normLen.first;
            auto len = normLen.second;
            
            auto refPt = points.front();
            
            auto p1 = refPt + norm * len * halfThickness;
            auto p2 = refPt + norm * -len * halfThickness;
            
            vertexObj->addVertex( cvglVertex({
                static_cast<float>((p1.x - halfScreenW) / halfScreenW),
                static_cast<float>(-(p1.y - halfScreenH) / halfScreenH)
            }));
            
            vertexObj->addVertex( cvglVertex({
                static_cast<float>((p2.x - halfScreenW) / halfScreenW),
                static_cast<float>(-(p2.y - halfScreenH) / halfScreenH)
            }));
        }
        
        vertexObj->endObj();
    }
    
    
    
    template<typename ptT, typename outT>
    void pointMatToXYBundle(const Mat& points, MapOSC& b, const float halfScreenW, const float halfScreenH )
    {

        vector<outT> x, y;
        for(int i = 0; i < points.rows; i++)
        {
            const ptT* Mi = points.ptr<ptT>(i);
            for(int j = 0; j < points.cols; j++)
            {
                const ptT& Mij = Mi[j];
                x.emplace_back( static_cast<float>((Mij.x - halfScreenW) / halfScreenW) );
                y.emplace_back( static_cast<float>((Mij.x - halfScreenW) / halfScreenW) );
            }
        }
        b.addMessage("/x", x);
        b.addMessage("/y", y);
        
    }
    
    void pointMatToXYBundle(const Mat& points, MapOSC& b, const float halfScreenW, const float halfScreenH )
    {
        switch( points.depth() )
        {
            case CV_32S:
                pointMatToXYBundle<Point2i,int>(points, b, halfScreenW, halfScreenH );
                break;
            case CV_32F:
                pointMatToXYBundle<Point2f,float>(points, b, halfScreenW, halfScreenH );
                break;
            case CV_64F:
                pointMatToXYBundle<Point2d,double>(points, b, halfScreenW, halfScreenH );
                break;
            default:
                cout << __func__ << "unknown type, maybe long? "  << endl;
                break;
        }
    }
    
    
    void rotatedRectToVertex(const RotatedRect& rect, cvglObject& vertexObj, const float halfScreenW, const float halfScreenH )
    {
        
        Point2f rectPts[4];
        rect.points( rectPts );

        vertexObj.newObj();
        for(int i = 0; i < 4; i++)
        {
            vertexObj.addVertex( cvglVertex({
                static_cast<float>((rectPts[i].x - halfScreenW) / halfScreenW),
                static_cast<float>(-(rectPts[i].y - halfScreenH) / halfScreenH)
            }));
        }
        vertexObj.endObj();
    }
    
    void rotatedRectToVertex(const RotatedRect& rect, unique_ptr<cvglObject>& vertexObj, const float halfScreenW, const float halfScreenH )
    {
        
        Point2f rectPts[4];
        rect.points( rectPts );
        
        vertexObj->newObj();
        for(int i = 0; i < 4; i++)
        {
            vertexObj->addVertex( cvglVertex({
                static_cast<float>((rectPts[i].x - halfScreenW) / halfScreenW),
                static_cast<float>(-(rectPts[i].y - halfScreenH) / halfScreenH)
            }));
        }
        vertexObj->endObj();
    }
    
    cv::Point getNormal( cv::Point p1, cv::Point p2 )
    {
//  Vec2f line = p1 - p0
//  Vec2f normal = Vec2f( -line.y, line.x).normalized().

        cv::Point delta = p2 - p1;
        cv::Point normal(-delta.y, delta.x);
        /*
        double vX = p2.x-p1.x;
        double vY = p2.y-p1.y;

        if(vX == 0 || vY == 0)
            return cv::Point();
        
        // normalize
        double mag = sqrt(vX*vX + vY*vY);
        return cv::Point( vX/mag, vY/mag );
         */
        
        return normal / sqrt(normal.x*normal.x + normal.y*normal.y );
    }
    
    


}

