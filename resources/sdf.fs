#version 100
precision mediump float;

uniform float u_time;
uniform vec2 u_resolution;

uniform int u_shapeType0;
uniform int u_shapeType1;
uniform vec2 u_shapePos0;
uniform vec2 u_shapePos1;
uniform float u_shapeRot0;
uniform float u_shapeRot1;
uniform float u_shapeScale0;
uniform float u_shapeScale1;
uniform float u_shapeMargin0;
uniform float u_shapeMargin1;

float dot2(vec2 x)
{
    return dot(x, x);
}

vec2 rotate(vec2 p, float a)
{
    return mat2(cos(a), -sin(a), sin(a), cos(a)) * p;
}

float sdHeart(vec2 p)
{
    p.x = abs(p.x);
    p.y += 0.5;

    if (p.y + p.x > 1.0)
    {
        return sqrt(dot2(p - vec2(0.25, 0.75))) - sqrt(2.0) / 4.0;        
    }
    
    return sqrt(min(dot2(p - vec2(0.00, 1.00)),
                    dot2(p - 0.5 * max(p.x + p.y, 0.0)))) * sign(p.x - p.y);
}

float sdCross(vec2 p)
{
    float w = 1.0;
    float r = 0.1;
    p = abs(p);
    return length(p - min(p.x + p.y, w) * 0.5) - r;
}

float sdChamferBox(vec2 p)
{
    vec2 b = vec2(0.5, 0.5);
    p = abs(p) - b;

    p = (p.y>p.x) ? p.yx : p.xy;
    p.y += 0.1;
    
    float k = 1.0-sqrt(2.0);
    if( p.y<0.0 && p.y+p.x*k<0.0) { return p.x; }
    if( p.x<p.y ) return (p.x+p.y)*sqrt(0.5);    
    return length(p);
}

float sdCircle(vec2 p)
{
    return length(p) - 0.5;
}

float sdStadium(vec2 p)
{
    vec2 a = vec2(0.0, +0.5);
    vec2 b = vec2(0.0, -0.5);
    
    vec2 pa = p - a;
    vec2 ba = b - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - (ba * h)) - 0.25;
}

float sdTriangle(vec2 p)
{
    float r = 0.5;
    float k = sqrt(3.0);
    p.x = abs(p.x) - r;
    p.y = p.y + r/k;
    if( p.x+k*p.y>0.0 ) p = vec2(p.x-k*p.y,-k*p.x-p.y)/2.0;
    p.x -= clamp( p.x, -2.0*r, 0.0 );
    return -length(p)*sign(p.y);
}

float sdPentagram(vec2 p)
{
    float r = 0.5;
    float k1x = 0.809016994;
    float k2x = 0.309016994;
    float k1y = 0.587785252;
    float k2y = 0.951056516;
    float k1z = 0.726542528;
    vec2  v1  = vec2( k1x,-k1y);
    vec2  v2  = vec2(-k1x,-k1y);
    vec2  v3  = vec2( k2x,-k2y);
    
    p.x = abs(p.x);
    p -= 2.0*max(dot(v1,p),0.0)*v1;
    p -= 2.0*max(dot(v2,p),0.0)*v2;
    p.x = abs(p.x);
    p.y -= r;
    return length(p-v3*clamp(dot(p,v3),0.0,k1z*r))
           * sign(p.y*v3.x-p.x*v3.y);
}

float sdArc(vec2 p)
{
    vec2 sc = vec2(sin(2.5), cos(2.5));
    float ra = 0.5;
    float rb = 0.1;
    
    p.x = abs(p.x);
    return ((sc.y*p.x>sc.x*p.y) ? length(p-sc*ra) : 
                                  abs(length(p)-ra)) - rb;
}

float sdHorseshoe(vec2 p)
{
    vec2 c = vec2(cos(1.3),sin(1.3));
    float r = 0.5;
    vec2 w = vec2(0.5,0.25);
    
    p.x = abs(p.x);
    float l = length(p);
    p = mat2(-c.x, c.y, c.y, c.x)*p;
    p = vec2((p.y>0.0 || p.x>0.0)?p.x:l*sign(-c.x),
             (p.x>0.0)?p.y:l );
    p = vec2(p.x,abs(p.y-r))-w;
    return length(max(p,0.0)) + min(0.0,max(p.x,p.y));
}

float sdMoon(vec2 p)
{
    float ra = 1.0;
    float rb = 0.8;
    float d = 0.5;
    
    p.y = abs(p.y);
    float a = (ra*ra - rb*rb + d*d)/(2.0*d);
    float b = sqrt(max(ra*ra-a*a,0.0));
    if( d*(p.x*b-p.y*a) > d*d*max(b-p.y,0.0) )
          return length(p-vec2(a,b));
    return max( (length(p          )-ra),
               -(length(p-vec2(d,0))-rb));
}

float sdfCoolS(vec2 p)
{
    float six = (p.y<0.0) ? -p.x : p.x;
    p.x = abs(p.x);
    p.y = abs(p.y) - 0.2;
    float rex = p.x - min(floor(p.x/0.4 + 0.5),0.4);
    float aby = abs(p.y-0.2)-0.6;
    
    float d = dot2(vec2(six,-p.y)-clamp(0.5*(six-p.y),0.0,0.2));
    d = min(d,dot2(vec2(p.x,-aby)-clamp(0.5*(p.x-aby),0.0,0.4)));
    d = min(d,dot2(vec2(rex,p.y  -clamp(p.y          ,0.0,0.4))));
    
    float s = 2.0*p.x + aby + abs(aby+0.4) - 0.4;
    return sqrt(d) * sign(s);
}

float sdShape0(vec2 p)
{
    p = rotate(p, u_shapeRot0);
    p -= rotate(u_shapePos0, u_shapeRot0);
    p /= u_shapeScale0;
    float d = 0.0;
    if      (u_shapeType0 == 0) { d = sdHeart(p); }
    else if (u_shapeType0 == 1) { d = sdCross(p); }
    else if (u_shapeType0 == 2) { d = sdChamferBox(p); }
    else if (u_shapeType0 == 3) { d = sdCircle(p); }
    else if (u_shapeType0 == 4) { d = sdStadium(p); }
    else if (u_shapeType0 == 5) { d = sdTriangle(p); }
    else if (u_shapeType0 == 6) { d = sdPentagram(p); }
    else if (u_shapeType0 == 7) { d = sdArc(p); }
    else if (u_shapeType0 == 8) { d = sdHorseshoe(p); }
    else if (u_shapeType0 == 9) { d = sdMoon(p); }
    else if (u_shapeType0 == 10) { d = sdfCoolS(p); }
    return d * u_shapeScale0 - u_shapeMargin0;
}

float sdShape1(vec2 p)
{
    p = rotate(p, u_shapeRot1);
    p -= rotate(u_shapePos1, u_shapeRot1);
    p /= u_shapeScale1;
    float d = 0.0;
    if      (u_shapeType1 == 0) { d = sdHeart(p); }
    else if (u_shapeType1 == 1) { d = sdCross(p); }
    else if (u_shapeType1 == 2) { d = sdChamferBox(p); }
    else if (u_shapeType1 == 3) { d = sdCircle(p); }
    else if (u_shapeType1 == 4) { d = sdStadium(p); }
    else if (u_shapeType1 == 5) { d = sdTriangle(p); }
    else if (u_shapeType1 == 6) { d = sdPentagram(p); }
    else if (u_shapeType1 == 7) { d = sdArc(p); }
    else if (u_shapeType1 == 8) { d = sdHorseshoe(p); }
    else if (u_shapeType1 == 9) { d = sdMoon(p); }
    else if (u_shapeType1 == 10) { d = sdfCoolS(p); }
    return d * u_shapeScale1 - u_shapeMargin1;
}

vec3 colorMap(float d0, float d1)
{
    float thickness = 0.005;
    
    vec3 ocol0 = vec3(0.75,0.0,0.0);
    vec3 icol0 = vec3(0.75,0.25,0.25);
    
    vec3 ocol1 = vec3(0.0,0.0,0.75);
    vec3 icol1 = vec3(0.25,0.25,0.75);
    
    ocol0 = mix(ocol0, vec3(1.0), 1.0 - smoothstep(0.0, thickness, max(d0, d1)));
    icol0 = mix(icol0, vec3(1.0), 1.0 - smoothstep(0.0, thickness, max(d0, d1)));
    ocol1 = mix(ocol1, vec3(1.0), 1.0 - smoothstep(0.0, thickness, max(d0, d1)));
    icol1 = mix(icol1, vec3(1.0), 1.0 - smoothstep(0.0, thickness, max(d0, d1)));

    vec3 col0 = (d0>0.0) ? ocol0 : icol0;
	col0 *= 1.0 - exp2(-20.0*abs(d0));
	col0 *= 0.8 + 0.2*cos(120.0*abs(d0));
	col0 = mix(col0, icol0, 1.0 - smoothstep(0.0, thickness, abs(d0)));
    
    if (u_shapeMargin0 > 0.0)
    {
        col0 = mix(col0, icol0 * 0.75, 1.0 - smoothstep(0.0, thickness, abs(d0 + u_shapeMargin0)));
    }
    
    col0 *= smoothstep(0.5, 0.0, abs(d0));
    
    vec3 col1 = (d1>0.0) ? ocol1 : icol1;
	col1 *= 1.0 - exp2(-20.0*abs(d1));
	col1 *= 0.8 + 0.2*cos(120.0*abs(d1));
	col1 = mix(col1, icol1, 1.0 - smoothstep(0.0, thickness, abs(d1)));
    
    if (u_shapeMargin1 > 0.0)
    {
        col1 = mix(col1, icol1 * 0.75, 1.0 - smoothstep(0.0, thickness, abs(d1 + u_shapeMargin1)));
    }

    col1 *= smoothstep(0.5, 0.0, abs(d1));
    
    return max(col0, col1);
}

void main()
{    
	vec2 p = (2.0*gl_FragCoord.xy-u_resolution)/u_resolution.y;
    
	gl_FragColor = vec4(colorMap(sdShape0(p), sdShape1(p)), 1.0);
}