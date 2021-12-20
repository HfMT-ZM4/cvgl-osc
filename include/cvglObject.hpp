#pragma once
#include <iostream>
#include <unordered_map>
#include "cvglTexture.hpp"
#include "cvglVAO.hpp"
#include "cvglVertex.hpp"


class cvglObject
{
public:
   
    void clear();
    void reserve(size_t n);
    size_t getSize(int index = 0);

    void newObj();
    void newObj(int drawType);
    void newObj(std::vector<int> drawTypes);
    void addVertex(cvglVertex v);
    void endObj();

    void setTextureID(GLuint texID );
    void setDrawTypes(std::vector<int> types);
    
    void bind();
    void unbind();
    void draw();
    void draw( std::vector<int> drawtypes );
    void draw( int drawtype );
    void initStaticDraw();
    
    void triangulate();
    
    void addVertexIndexArray( std::vector<GLuint> idx );
    void rewriteVerticesWithIds( std::vector<GLuint> idx );
    
    inline cvglVertex getVertex(int idx){ return m_vertices[idx]; }
    
    inline void setTexCord( int idx, float u, float v ){
        m_vertices[idx].texcoord[0] = u;
        m_vertices[idx].texcoord[1] = v;
    }
    
    inline void setPosition( int idx, float x, float y, float z )
    {
        m_vertices[idx].position[0] = x;
        m_vertices[idx].position[1] = y;
        m_vertices[idx].position[2] = z;
    }

    inline void setVertex( int idx, cvglVertex vert )
    {
        m_vertices[idx] = vert;
    }

private:
    cvglVAO m_VAO;
    GLuint m_draw_mode = GL_STREAM_DRAW;
    
    std::vector<cvglVertex> m_vertices;
    std::vector<GLuint> m_idx;
    std::vector<size_t> m_idx_start;
    std::vector<GLuint> m_idx_size;

    std::vector<size_t> m_start;
    std::vector<size_t> m_size;
    std::vector<GLuint> m_tex;
    
    std::vector< std::vector<int> > m_drawTypes;
    
    using cvglPoint = std::tuple<double,double>;
    using cvglPolygon = std::vector<cvglPoint>;
    
    std::unordered_map< size_t, cvglPolygon > m_vert_list; // verticies in list format, separated for triangulation
    std::unordered_map< size_t, std::vector<GLuint> > m_indices; // element buffer per object

    bool m_useElementArray = false;
        
};
