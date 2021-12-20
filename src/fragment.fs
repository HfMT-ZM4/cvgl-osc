#version 330 core

in vec2 Texcoord;

uniform float gamma;
uniform float contrast;
uniform float saturation;
uniform float brightness;
uniform float scale_alpha;


uniform vec4 vignette_xyr_aspect;
uniform float time;

uniform sampler2D tex;
 float thresh_w = 0.9;
 float thresh_b = 0.1;

out vec4 outColor;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

const float PHI = 1.61803398874989484820459; // Φ = Golden Ratio

float gold_noise(vec2 xy, float seed)
{
    return fract(tan(distance(xy*PHI, xy)*seed)*xy.x);
}


mat4 brightnessMatrix( float brightness )
{
    return mat4( 1, 0, 0, 0,
                 0, 1, 0, 0,
                 0, 0, 1, 0,
                 brightness, brightness, brightness, 1 );
}

mat4 contrastMatrix( float contrast )
{
    float t = ( 1.0 - contrast );// / 2.0;

    return mat4( contrast, 0, 0, 0,
                 0, contrast, 0, 0,
                 0, 0, contrast, 0,
                 1, 1, 1, 1 );

}

mat4 saturationMatrix( float saturation )
{
    vec3 luminance = vec3( 0.3086, 0.6094, 0.0820 );

    float oneMinusSat = 1.0 - saturation;

    vec3 red = vec3( luminance.x * oneMinusSat );
    red+= vec3( saturation, 0, 0 );

    vec3 green = vec3( luminance.y * oneMinusSat );
    green += vec3( 0, saturation, 0 );

    vec3 blue = vec3( luminance.z * oneMinusSat );
    blue += vec3( 0, 0, saturation );

    return mat4( red,     0,
                 green,   0,
                 blue,    0,
                 0, 0, 0, 1 );
}


vec4 brightnessContrast(vec4 value, float brightness, float contrast)
{
    return vec4((value.rgb - 0.5) * contrast + 0.5 + brightness, value.a);
}

vec4 czm_saturation(vec4 value, float adjustment)
{
    // Algorithm from Chapter 16 of OpenGL Shading Language
    const vec3 W = vec3(0.2125, 0.7154, 0.0721);
    vec3 intensity = vec3(dot(value.rgb, W));
    return vec4( mix(intensity, value.rgb, adjustment), value.a);
}

vec4 gammaCorrection(vec4 value, float param)
{
    return vec4( pow( value.rgb, vec3(1. / param)), value.a);
}

void main()
{

    vec4 tex_samp = texture( tex, Texcoord );

    tex_samp = brightnessContrast(tex_samp, brightness, contrast);

    tex_samp = czm_saturation(tex_samp, saturation);

    tex_samp = gammaCorrection(tex_samp, gamma);

    tex_samp.a *= scale_alpha;
/*
    float luma = sqrt( 0.299*tex_samp.r*tex_samp.r + 0.587*tex_samp.g*tex_samp.g + 0.114*tex_samp.b*tex_samp.b );

    if( luma >= thresh_w )
        tex_samp = pow(tex_samp, vec4(0.2, 0.2, 0.2, 1) );
    else if ( luma <= thresh_b )
        tex_samp = pow(tex_samp, vec4(5, 5, 5, 1) );
*/
    if( vignette_xyr_aspect.z == 1 )
    {
        outColor = tex_samp;
    }
    else
    {
        float xfade = 0.05;
        float dx = vignette_xyr_aspect.x - Texcoord.x;
        float dy = vignette_xyr_aspect.y - (Texcoord.y / vignette_xyr_aspect.w);
        float dist = sqrt(dx*dx + dy*dy);

        if( dist < vignette_xyr_aspect.z )
        {
            outColor = tex_samp;
        }
        else if( (dist - vignette_xyr_aspect.z) <= xfade )
        {
            float fade = (dist - vignette_xyr_aspect.z) / xfade;

            outColor = tex_samp * vec4(1, 1, 1, 1 - fade * fade * fade);

        }
        else
        {
            outColor = vec4(0,0,0,1);
        }
    }


   /*
    outColor.r = gold_noise(Texcoord, Time*1.1);
    outColor.g = gold_noise(Texcoord, Time*1.7);
    outColor.b = gold_noise(Texcoord, Time*1.4);
    */
}
