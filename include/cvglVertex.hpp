#pragma once

// note: the shader has to match the vertex array format

struct cvglVertex {
    float position[3]; // layout 0
    float texcoord[2]; // layout 2
    
    /*
     cvglVertex(float x, float y, float z, float u, float v)
     {
     position = {x,y,z};
     texcord = {u,v};
     }
     
     cvglVertex(float x, float y, float z)
     {
     position = {x,y,z};
     texcord = {0,0};
     }*/
    
};
