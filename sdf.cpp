extern "C"
{
#include "raylib.h"
#define RAYMATH_STATIC_INLINE
#define RAYMATH_DISABLE_CPP_OPERATORS
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
}
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#include <stdio.h>
#include <assert.h>
#include <float.h>

#include <initializer_list>
#include <functional>

static int shapeType0;
static int shapeType1;
static Vector2 shapePos0;
static Vector2 shapePos1;
static float shapeRot0;
static float shapeRot1;
static float shapeScale0;
static float shapeScale1;
static float shapeMargin0;
static float shapeMargin1;

float signf(float x)
{
    return x > 0.0f ? 1.0f : x < 0.0f ? -1.0f : 0.0f;
}

float maxf(float x, float y)
{
    return x > y ? x : y;
}

float minf(float x, float y)
{
    return x < y ? x : y;
}

Vector2 Vector2Absolute(Vector2 v)
{
    return (Vector2){ fabsf(v.x), fabsf(v.y) };
}

Vector2 Vector2Flip(Vector2 v)
{
    return (Vector2){ v.y, v.x };
}

float Vector2Dot(Vector2 x, Vector2 y)
{
    return x.x * y.x + x.y * y.y;
}

float Vector2Dot2(Vector2 x)
{
    return Vector2Dot(x, x);
}

Vector2 Vector2SubtractFloat(Vector2 x, float y)
{
    x.x -= y;
    x.y -= y;
    return x;
}

float sdHeart(Vector2 p)
{
    p.x = fabsf(p.x);
    p.y += 0.5f;

    if (p.y + p.x > 1.0)
    {
        return sqrtf(Vector2Dot2(Vector2Subtract(p, (Vector2){ 0.25, 0.75 }))) - sqrtf(2.0) / 4.0;        
    }
    
    return sqrtf(minf(Vector2Dot2(Vector2Subtract(p, (Vector2){ 0.0, 1.0 })),
                      Vector2Dot2(Vector2SubtractFloat(p, 0.5 * maxf(p.x + p.y, 0.0))))) * signf(p.x - p.y);
}

float sdCross(Vector2 p)
{
    float w = 1.0f;
    float r = 0.1f;
    p = Vector2Absolute(p);
    return Vector2Length(Vector2SubtractFloat(p, minf(p.x + p.y, w) * 0.5)) - r;
}

float sdChamferBox(Vector2 p)
{
    Vector2 b = (Vector2){ 0.5f, 0.5f }; 
    float chamfer = 0.1f;
    p = Vector2Subtract(Vector2Absolute(p), b);

    p = (p.y > p.x) ? Vector2Flip(p) : p;
    p.y += chamfer;
    
    float k = 1.0f - sqrtf(2.0f);
    if (p.y < 0.0f && p.y + p.x * k < 0.0f) { return p.x; }
    if (p.x < p.y) { return (p.x + p.y) * sqrtf(0.5f); }
    return Vector2Length(p);
}

float sdCircle(Vector2 p)
{
    return Vector2Length(p) - 0.5f;
}

float sdStadium(Vector2 p)
{
    Vector2 a = { 0.0f, +0.5f };
    Vector2 b = { 0.0f, -0.5f };
    
    Vector2 pa = Vector2Subtract(p, a);
    Vector2 ba = Vector2Subtract(b, a);
    float h = Clamp(Vector2Dot(pa, ba) / Vector2Dot(ba, ba), 0.0f, 1.0f);
    return Vector2Length(Vector2Subtract(pa, Vector2Scale(ba, h))) - 0.25f;
}

float sdTriangle(Vector2 p)
{
    const float r = 0.5f;
    const float k = sqrtf(3.0f);
    p.x = fabsf(p.x) - r;
    p.y = p.y + r/k;
    if (p.x+k * p.y > 0.0f) { p = (Vector2){ (p.x-k*p.y) / 2.0f, (-k*p.x-p.y) / 2.0f }; }
    p.x -= Clamp(p.x, -2.0*r, 0.0f);
    return -Vector2Length(p) * signf(p.y);
}

float sdPentagram(Vector2 p)
{
    const float r = 0.5f;
    const float k1x = 0.809016994;
    const float k2x = 0.309016994;
    const float k1y = 0.587785252;
    const float k2y = 0.951056516;
    const float k1z = 0.726542528;
    const Vector2 v1  = (Vector2){  k1x, -k1y };
    const Vector2 v2  = (Vector2){ -k1x, -k1y };
    const Vector2 v3  = (Vector2){  k2x, -k2y };
    
    p.x = fabsf(p.x);
    p = Vector2Subtract(p, Vector2Scale(v1, 2.0*maxf(Vector2Dot(v1,p),0.0)));
    p = Vector2Subtract(p, Vector2Scale(v2, 2.0*maxf(Vector2Dot(v2,p),0.0)));
    p.x = fabsf(p.x);
    p.y -= r;
    return Vector2Length(Vector2Subtract(p, Vector2Scale(v3, Clamp(Vector2Dot(p,v3),0.0,k1z*r))))
           * signf(p.y*v3.x-p.x*v3.y);
}

float sdArc(Vector2 p)
{
    Vector2 sc = (Vector2){ sinf(2.5f), cosf(2.5f) };
    float ra = 0.5f;
    float rb = 0.1f;
    
    p.x = fabsf(p.x);
    return ((sc.y*p.x>sc.x*p.y) ? Vector2Length(Vector2Subtract(p, Vector2Scale(sc, ra))) : 
                                  fabsf(Vector2Length(p)-ra)) - rb;
}

float sdHorseshoe(Vector2 p)
{
    Vector2 c = (Vector2){ cosf(1.3f), sinf(1.3f) };
    float r = 0.5f;
    Vector2 w = (Vector2){ 0.5f, 0.25f };
    
    p.x = fabsf(p.x);
    float l = Vector2Length(p);
    p = (Vector2){ Vector2Dot((Vector2){ -c.x, c.y }, p), Vector2Dot((Vector2){ c.y, c.x }, p) };
    p = (Vector2){ (p.y>0.0f || p.x>0.0f)?p.x:l*signf(-c.x), (p.x>0.0f)?p.y:l };
    p = Vector2Subtract((Vector2){ p.x,fabsf(p.y-r) }, w);
    return Vector2Length((Vector2){ maxf(p.x,0.0f), maxf(p.y,0.0f)}) + minf(0.0f,maxf(p.x,p.y));
}

float sdMoon(Vector2 p)
{
    float ra = 1.0f;
    float rb = 0.8f;
    float d = 0.5f;
    
    p.y = fabsf(p.y);
    float a = (ra*ra - rb*rb + d*d)/(2.0f*d);
    float b = sqrtf(maxf(ra*ra-a*a,0.0f));
    if( d*(p.x*b-p.y*a) > d*d*maxf(b-p.y,0.0f) )
          return Vector2Length(Vector2Subtract(p, (Vector2){ a, b }));
    return maxf((Vector2Length(p          )-ra),
               -(Vector2Length(Vector2Subtract(p, (Vector2){ d, 0.0f }))-rb));
}

float sdfCoolS(Vector2 p)
{
    float six = (p.y<0.0f) ? -p.x : p.x;
    p.x = fabsf(p.x);
    p.y = fabsf(p.y) - 0.2f;
    float rex = p.x - minf(floorf(p.x/0.4f + 0.5f),0.4f);
    float aby = fabsf(p.y-0.2f)-0.6f;
    
    float d = Vector2Dot2((Vector2){ six-Clamp(0.5f*(six-p.y),0.0f,0.2f),-p.y-Clamp(0.5f*(six-p.y),0.0f,0.2f)});
    d = minf(d,Vector2Dot2((Vector2){p.x-Clamp(0.5f*(p.x-aby),0.0f,0.4f),-aby-Clamp(0.5f*(p.x-aby),0.0f,0.4f)}));
    d = minf(d,Vector2Dot2((Vector2){rex,p.y-Clamp(p.y       ,0.0f,0.4f)}));
    
    float s = 2.0f*p.x + aby + fabsf(aby+0.4f) - 0.4f;
    return sqrtf(d) * signf(s);
}

float sdShape0(Vector2 p)
{
    p = Vector2Rotate(p, -shapeRot0);
    p = Vector2Subtract(p, Vector2Rotate(shapePos0, -shapeRot0));
    p = Vector2Scale(p, 1.0f / shapeScale0);
    
    float d = 0.0f;
    if      (shapeType0 ==  0) { d = sdHeart(p); }
    else if (shapeType0 ==  1) { d = sdCross(p); }
    else if (shapeType0 ==  2) { d = sdChamferBox(p); }
    else if (shapeType0 ==  3) { d = sdCircle(p); }
    else if (shapeType0 ==  4) { d = sdStadium(p); }
    else if (shapeType0 ==  5) { d = sdTriangle(p); }
    else if (shapeType0 ==  6) { d = sdPentagram(p); }
    else if (shapeType0 ==  7) { d = sdArc(p); }
    else if (shapeType0 ==  8) { d = sdHorseshoe(p); }
    else if (shapeType0 ==  9) { d = sdMoon(p); }
    else if (shapeType0 == 10) { d = sdfCoolS(p); }
    return d * shapeScale0 - shapeMargin0;
}

float sdShape1(Vector2 p)
{
    p = Vector2Rotate(p, -shapeRot1);
    p = Vector2Subtract(p, Vector2Rotate(shapePos1, -shapeRot1));
    p = Vector2Scale(p, 1.0f / shapeScale1);
    
    float d = 0.0f;
    if      (shapeType1 ==  0) { d = sdHeart(p); }
    else if (shapeType1 ==  1) { d = sdCross(p); }
    else if (shapeType1 ==  2) { d = sdChamferBox(p); }
    else if (shapeType1 ==  3) { d = sdCircle(p); }
    else if (shapeType1 ==  4) { d = sdStadium(p); }
    else if (shapeType1 ==  5) { d = sdTriangle(p); }
    else if (shapeType1 ==  6) { d = sdPentagram(p); }
    else if (shapeType1 ==  7) { d = sdArc(p); }
    else if (shapeType1 ==  8) { d = sdHorseshoe(p); }
    else if (shapeType1 ==  9) { d = sdMoon(p); }
    else if (shapeType1 == 10) { d = sdfCoolS(p); }
    return d * shapeScale1 - shapeMargin1;
}

Vector2 normalShape0(Vector2 p)
{
    const float eps = 0.0001f;

    return Vector2Normalize((Vector2){
        sdShape0(Vector2Add(p, (Vector2){ eps, 0.0f })) - sdShape0(Vector2Subtract(p, (Vector2){ eps, 0.0f })),
        sdShape0(Vector2Add(p, (Vector2){ 0.0f, eps })) - sdShape0(Vector2Subtract(p, (Vector2){ 0.0f, eps })),
    });
}

Vector2 normalShape1(Vector2 p)
{
    const float eps = 0.0001f;

    return Vector2Normalize((Vector2){
        sdShape1(Vector2Add(p, (Vector2){ eps, 0.0f })) - sdShape1(Vector2Subtract(p, (Vector2){ eps, 0.0f })),
        sdShape1(Vector2Add(p, (Vector2){ 0.0f, eps })) - sdShape1(Vector2Subtract(p, (Vector2){ 0.0f, eps })),
    });
}

typedef struct Bound
{
    Vector2 minimum;
    Vector2 maximum;
} Bound;

Bound BoundTranslate(Bound a, Vector2 offset)
{
    return (Bound){
        .minimum = Vector2Add(a.minimum, offset),
        .maximum = Vector2Add(a.maximum, offset),
    };
}

Bound BoundScale(Bound a, float scale)
{
    return (Bound){
        .minimum = Vector2Scale(a.minimum, scale),
        .maximum = Vector2Scale(a.maximum, scale),
    };
}

Bound BoundRotate(Bound a, float angle)
{
    Vector2 v0 = Vector2Rotate((Vector2){ a.minimum.x, a.minimum.y }, angle);
    Vector2 v1 = Vector2Rotate((Vector2){ a.maximum.x, a.minimum.y }, angle);
    Vector2 v2 = Vector2Rotate((Vector2){ a.minimum.x, a.maximum.y }, angle);
    Vector2 v3 = Vector2Rotate((Vector2){ a.maximum.x, a.maximum.y }, angle);
    
    return (Bound){
        .minimum = Vector2Min(Vector2Min(Vector2Min(v0, v1), v2), v3),
        .maximum = Vector2Max(Vector2Max(Vector2Max(v0, v1), v2), v3),
    };
}

Bound BoundUnion(Bound a, Bound b)
{
    return (Bound){
        .minimum = Vector2Min(a.minimum, b.minimum),
        .maximum = Vector2Max(a.maximum, b.maximum),
    };
}

Bound BoundIntersection(Bound a, Bound b)
{
    return (Bound){
        .minimum = Vector2Max(a.minimum, b.minimum),
        .maximum = Vector2Min(a.maximum, b.maximum),
    };
}

void BoundSplit(Bound * b0, Bound * b1, Bound a)
{
    float w = a.maximum.x - a.minimum.x;
    float h = a.maximum.y - a.minimum.y;
    
    if (w > h)
    {
        *b0 = (Bound){
            .minimum = (Vector2){ a.minimum.x           , a.minimum.y },
            .maximum = (Vector2){ a.minimum.x + w / 2.0f, a.maximum.y },
        };
        
        *b1 = (Bound){
            .minimum = (Vector2){ a.minimum.x + w / 2.0f, a.minimum.y },
            .maximum = (Vector2){ a.maximum.x,            a.maximum.y },
        };
    }
    else
    {
        *b0 = (Bound){
            .minimum = (Vector2){ a.minimum.x, a.minimum.y            },
            .maximum = (Vector2){ a.maximum.x, a.minimum.y + h / 2.0f },
        };
        
        *b1 = (Bound){
            .minimum = (Vector2){ a.minimum.x, a.minimum.y + h / 2.0f },
            .maximum = (Vector2){ a.maximum.x, a.maximum.y,           },
        }; 
    }
}


bool BoundIsValid(Bound x)
{
    return x.minimum.x <= x.maximum.x && x.minimum.y <= x.maximum.y;
}

Vector2 BoundCenter(Bound a)
{
    return Vector2Scale(Vector2Add(a.minimum, a.maximum), 0.5f);
}

float BoundRadius(Bound a)
{
    float hw = (a.maximum.x - a.minimum.x) / 2.0f;
    float hh = (a.maximum.y - a.minimum.y) / 2.0f;
    return sqrtf(hw*hw + hh*hh);
}

Bound BoundExpand(Bound b, Vector2 margin)
{
    return (Bound){
        (Vector2){ b.minimum.x - margin.x, b.minimum.y - margin.y },
        (Vector2){ b.maximum.x + margin.x, b.maximum.y + margin.y },
    };
}

Bound boundHeart()
{
    float r = 0.125f;
    return (Bound){ (Vector2){ -0.5f - r, -0.5f }, (Vector2){ +0.5f + r, 0.5f + r } }; 
}

Bound boundCross()
{
    float r = 0.1f;
    return (Bound){ (Vector2){ -0.5f - r, -0.5f - r }, (Vector2){ +0.5f + r, +0.5f + r } };
}

Bound boundChamferBox()
{
    return (Bound){ (Vector2){ -0.5f, -0.5f }, (Vector2){ +0.5f, +0.5f } };
}

Bound boundCircle()
{
    return (Bound){ (Vector2){ -0.5f, -0.5f }, (Vector2){ +0.5f, +0.5f } };
}

Bound boundStadium()
{
    return (Bound){ (Vector2){ -0.25f, -0.75f }, (Vector2){ +0.25f, +0.75f } };
}

Bound boundTriangle()
{
    return (Bound){ (Vector2){ -0.5f, -0.5f }, (Vector2){ +0.5f, +0.5f } };
}

Bound boundPentagram()
{
    return (Bound){ (Vector2){ -0.5f, -0.5f }, (Vector2){ +0.5, +0.5f } };
}

Bound boundArc()
{
    return (Bound){ (Vector2){ -0.6f, -0.5f }, (Vector2){ +0.6, +0.6f } };
}

Bound boundHorseshoe()
{
    return (Bound){ (Vector2){ -0.75f, -0.75f }, (Vector2){ +0.75, +0.75f } };
}

Bound boundMoon()
{
    return (Bound){ (Vector2){ -1.0f, -1.0f }, (Vector2){ +0.75, +1.0f } };
}

Bound boundCoolS()
{
    return (Bound){ (Vector2){ -0.5f, -1.0f }, (Vector2){ +0.5, +1.0f } };
}

Bound boundShape0()
{
    Bound bound = { 0 };
    if      (shapeType0 ==  0) { bound = boundHeart(); }
    else if (shapeType0 ==  1) { bound = boundCross(); }
    else if (shapeType0 ==  2) { bound = boundChamferBox(); }
    else if (shapeType0 ==  3) { bound = boundCircle(); }
    else if (shapeType0 ==  4) { bound = boundStadium(); }
    else if (shapeType0 ==  5) { bound = boundTriangle(); }
    else if (shapeType0 ==  6) { bound = boundPentagram(); }
    else if (shapeType0 ==  7) { bound = boundArc(); }
    else if (shapeType0 ==  8) { bound = boundHorseshoe(); }
    else if (shapeType0 ==  9) { bound = boundMoon(); }
    else if (shapeType0 == 10) { bound = boundCoolS(); }
    
    bound = BoundScale(bound, shapeScale0);
    bound = BoundExpand(bound, (Vector2){ shapeMargin0, shapeMargin0 });
    
    return BoundTranslate(BoundRotate(bound, shapeRot0), shapePos0);
}

Bound boundShape1()
{
    Bound bound = { 0 };
    if      (shapeType1 ==  0) { bound = boundHeart(); }
    else if (shapeType1 ==  1) { bound = boundCross(); }
    else if (shapeType1 ==  2) { bound = boundChamferBox(); }
    else if (shapeType1 ==  3) { bound = boundCircle(); }
    else if (shapeType1 ==  4) { bound = boundStadium(); }
    else if (shapeType1 ==  5) { bound = boundTriangle(); }
    else if (shapeType1 ==  6) { bound = boundPentagram(); }
    else if (shapeType1 ==  7) { bound = boundArc(); }
    else if (shapeType1 ==  8) { bound = boundHorseshoe(); }
    else if (shapeType1 ==  9) { bound = boundMoon(); }
    else if (shapeType1 == 10) { bound = boundCoolS(); }
    
    bound = BoundScale(bound, shapeScale1);
    bound = BoundExpand(bound, (Vector2){ shapeMargin1, shapeMargin1 });

    return BoundTranslate(BoundRotate(bound, shapeRot1), shapePos1);
}

Rectangle BoundScreenRectangle(Bound b)
{
    Bound sb = BoundScale(BoundTranslate(BoundScale(b, GetScreenHeight()), (Vector2){ (float)GetScreenWidth(), (float)GetScreenHeight() }), 0.5f);
    
    float width = roundf(sb.maximum.x - sb.minimum.x);
    float height = roundf(sb.maximum.y - sb.minimum.y);
    float posx = roundf(sb.minimum.x);
    float posy = GetScreenHeight() - height - roundf(sb.minimum.y);
    
    return (Rectangle){ posx, posy, width, height };
}

void DrawBound(Bound b, Color color, float thickness)
{
    DrawRectangleLinesEx(BoundScreenRectangle(b), thickness, color);
}

void DrawSample(Vector2 p, Color color, float radius)
{
    Vector2 sp = Vector2Scale(Vector2Add(Vector2Scale(p, GetScreenHeight()), (Vector2){ (float)GetScreenWidth(), (float)GetScreenHeight() }), 0.5f);
    
    float posx = roundf(sp.x);
    float posy = roundf(sp.y);
    
    DrawCircleV(
        (Vector2){ posx, GetScreenHeight() - posy },
        radius,
        color);
}

void DrawBoundRadius(Vector2 p, Color color, float radius)
{
    Vector2 sp = Vector2Scale(Vector2Add(Vector2Scale(p, GetScreenHeight()), (Vector2){ (float)GetScreenWidth(), (float)GetScreenHeight() }), 0.5f);
    
    float posx = roundf(sp.x);
    float posy = roundf(sp.y);
    
    DrawCircleLinesV(
        (Vector2){ posx, GetScreenHeight() - posy },
        radius * GetScreenHeight() * 0.5f,
        color);
}

void DrawSurfacePoint(Vector2 start, Vector2 normal, float thick, float headSize, Color color)
{
    Vector2 end = Vector2Add(start, Vector2Scale(normal, 0.05f));
    
    start = Vector2Scale(Vector2Add(Vector2Scale(start, GetScreenHeight()), (Vector2){ (float)GetScreenWidth(), (float)GetScreenHeight() }), 0.5f);
    end = Vector2Scale(Vector2Add(Vector2Scale(end, GetScreenHeight()), (Vector2){ (float)GetScreenWidth(), (float)GetScreenHeight() }), 0.5f);
    
    start.y = GetScreenHeight() - start.y;
    end.y = GetScreenHeight() - end.y;
    
    // Draw the main shaft
    DrawLineEx(start, end, thick, color);

    // Calculate angle of the line
    float angle = atan2f(end.y - start.y, end.x - start.x);

    // Calculate the three points of the arrowhead triangle
    Vector2 tip = end;
    Vector2 left = {
        end.x - headSize * cosf(angle - M_PI / 6.0f),
        end.y - headSize * sinf(angle - M_PI / 6.0f)
    };
    Vector2 right = {
        end.x - headSize * cosf(angle + M_PI / 6.0f),
        end.y - headSize * sinf(angle + M_PI / 6.0f)
    };

    // Draw solid arrowhead
    DrawTriangle(left, tip, right, color);
}

////////////////////////////

static int sdSampleNum = 0;

bool sdIntersects(Bound b)
{
    // Compute the bound center and radius
    Vector2 p = BoundCenter(b);
    float r = BoundRadius(b);
    DrawBoundRadius(p, Fade(GREEN, 0.33f), r);
    DrawBound(b, Fade(GREEN, 0.33f), 1.0f);
    DrawSample(p, Fade(GREEN, 0.33f), 1.5f);
    
    // Sample each of the SDFs at the bound center
    float d0 = sdShape0(p);
    float d1 = sdShape1(p);
    sdSampleNum++;
    
    // If the sampled point contains an intersection return true
    if (maxf(d0, d1) <= 0.0f)
    {
        return true;
    }
    
    // If there is no possible intersection or the radius is too small return false
    float minrad = 0.02f;
    if (maxf(d0, d1) - r > 0.0f || r < minrad)
    {
        return false;
    }
    
    // Otherwise split the Bounding Box in two and recurse
    Bound s0, s1;
    BoundSplit(&s0, &s1, b);
    return sdIntersects(s0) || sdIntersects(s1);
}

enum
{
    // Maximum queue size
    INTERSECTS_QUEUE_MAX = 128,
};

bool sdIntersects2(Bound b)
{
    // Insert the initial bound into the queue
    Bound queue[INTERSECTS_QUEUE_MAX];
    int queue_num = 1;
    int queue_start = 0;
    queue[queue_start] = b;
    
    // Iterate over the queue
    while (queue_num)
    {
        // Process the next bound in the queue
        Bound c = queue[queue_start];
        queue_start = (queue_start + 1) % INTERSECTS_QUEUE_MAX;
        queue_num--;
        
        // Compute the bound center and radius
        Vector2 p = BoundCenter(c);
        float r = BoundRadius(c);
        DrawBoundRadius(p, Fade(GREEN, 0.33f), r);
        DrawBound(c, Fade(GREEN, 0.33f), 1.0f);
        DrawSample(p, Fade(GREEN, 0.33f), 1.5f);
     
        // Sample the SDFs at the bound center
        float d0 = sdShape0(p);
        float d1 = sdShape1(p);
        sdSampleNum++;
        
        // If the sampled point contains an intersection return true
        if (maxf(d0, d1) <= 0.0f)
        {
            return true;
        }
        
        // If the bound contains a possible intersection, 
        // the radius is large enough, and the queue is not full
        float minrad = 0.02f;
        if (maxf(d0, d1) - r > 0.0f 
        || r < minrad 
        || queue_num + 2 > INTERSECTS_QUEUE_MAX)
        {
            continue;
        }
        
        // Split the bound into two add to the queue
        Bound s0, s1;
        BoundSplit(&s0, &s1, c);
        queue[(queue_start + queue_num + 0) % INTERSECTS_QUEUE_MAX] = s0;
        queue[(queue_start + queue_num + 1) % INTERSECTS_QUEUE_MAX] = s1;
        queue_num += 2;
    }
    
    // No bounds left to search, return false
    return false;
}

////////////////////////////

void sdPenetration(
    Vector2 * point,
    float * depth,
    Bound b)
{
    // Compute the bound center and radius
    Vector2 p = BoundCenter(b);
    float r = BoundRadius(b);
    DrawBoundRadius(p, Fade(GREEN, 0.33f), r);
    DrawBound(b, Fade(GREEN, 0.33f), 1.0f);
    DrawSample(p, Fade(GREEN, 0.33f), 1.5f);

    // Sample each of the SDFs at the bound center
    float d0 = sdShape0(p);
    float d1 = sdShape1(p);
    sdSampleNum++;
    
    // Update best depth found
    if (maxf(d0, d1) <= 0.0f && d0 + d1 <= *depth)
    {
        *depth = d0 + d1;
        *point = p;
    }
    
    // There is no possible intersection or improvement
    float minrad = 0.02f;
    if (maxf(d0, d1) - r > 0.0f || d0 + d1 - 2 * r > *depth || r < minrad)
    {
        return;
    }
    
    Bound s0, s1;
    BoundSplit(&s0, &s1, b);
    sdPenetration(point, depth, s0);
    sdPenetration(point, depth, s1);
}

void sdPenetration2(
    Vector2 * point,
    float * depth,
    Bound b)
{
    // Insert the initial bound into the queue
    Bound queue[INTERSECTS_QUEUE_MAX];
    int queue_num = 1;
    int queue_start = 0;
    queue[queue_start] = b;
    
    // Iterate over the queue
    while (queue_num)
    {
        // Process the next bound in the queue
        Bound c = queue[queue_start];
        queue_start = (queue_start + 1) % INTERSECTS_QUEUE_MAX;
        queue_num--;
        
        // Compute the bound center and radius
        Vector2 p = BoundCenter(c);
        float r = BoundRadius(c);
        DrawBoundRadius(p, Fade(GREEN, 0.33f), r);
        DrawBound(c, Fade(GREEN, 0.33f), 1.0f);
        DrawSample(p, Fade(GREEN, 0.33f), 1.5f);

        // Sample each of the SDFs at the bound center
        float d0 = sdShape0(p);
        float d1 = sdShape1(p);
        sdSampleNum++;
        
        // Update best depth found
        if (maxf(d0, d1) <= 0.0f && d0 + d1 <= *depth)
        {
            *depth = d0 + d1;
            *point = p;
        }
        
        // There is no possible intersection or improvement
        float minrad = 0.02f;
        if (maxf(d0, d1) - r > 0.0f 
        || d0 + d1 - 2 * r > *depth 
        || r < minrad 
        || queue_num + 2 > INTERSECTS_QUEUE_MAX)
        {
            continue;
        }
        
        Bound s0, s1;
        BoundSplit(&s0, &s1, c);
        queue[(queue_start + queue_num + 0) % INTERSECTS_QUEUE_MAX] = s0;
        queue[(queue_start + queue_num + 1) % INTERSECTS_QUEUE_MAX] = s1;
        queue_num += 2;
    }
}

////////////////////////////

bool sdDistance(
    Vector2 * point,
    float * dist,
    Bound b)
{
    // Compute the bound center and radius
    Vector2 p = BoundCenter(b);
    float r = BoundRadius(b);
    DrawBoundRadius(p, Fade(GREEN, 0.33f), r);
    DrawBound(b, Fade(GREEN, 0.33f), 1.0f);
    DrawSample(p, Fade(GREEN, 0.33f), 1.5f);

    // Sample each of the SDFs at the bound center
    float d0 = sdShape0(p);
    float d1 = sdShape1(p);
    sdSampleNum++;
    
    // If the sampled point contains an intersection return true
    if (maxf(d0, d1) <= 0.0f)
    {
        *dist = 0.0f;
        *point = p;
        return true;
    }
    
    // Update smallest combined dist found
    if (minf(d0, d1) > 0.0f && d0 + d1 <= *dist)
    {
        *dist = d0 + d1;
        *point = p;
    }
    
    // There is no possible improvement
    float minrad = 0.02f;
    if (minf(d0, d1) + r <= 0.0f || d0 + d1 - 2 * r > *dist || r < minrad)
    {
        return false;
    }
    
    Bound s0, s1;
    BoundSplit(&s0, &s1, b);
    return sdDistance(point, dist, s0) || sdDistance(point, dist, s1);
}


bool sdDistance2(
    Vector2 * point,
    float * dist,
    Bound b)
{
    // Insert the initial bound into the queue
    Bound queue[INTERSECTS_QUEUE_MAX];
    int queue_num = 1;
    int queue_start = 0;
    queue[queue_start] = b;
    
    // Iterate over the queue
    while (queue_num)
    {
        // Process the next bound in the queue
        Bound c = queue[queue_start];
        queue_start = (queue_start + 1) % INTERSECTS_QUEUE_MAX;
        queue_num--;
        
        // Compute the bound center and radius
        Vector2 p = BoundCenter(c);
        float r = BoundRadius(c);
        DrawBoundRadius(p, Fade(GREEN, 0.33f), r);
        DrawBound(c, Fade(GREEN, 0.33f), 1.0f);
        DrawSample(p, Fade(GREEN, 0.33f), 1.5f);

        // Sample each of the SDFs at the bound center
        float d0 = sdShape0(p);
        float d1 = sdShape1(p);
        sdSampleNum++;
        
        // If the sampled point contains an intersection return true
        if (maxf(d0, d1) <= 0.0f)
        {
            *dist = 0.0f;
            *point = p;
            return true;
        }
        
        // Update smallest combined dist found
        if (minf(d0, d1) > 0.0f && d0 + d1 <= *dist)
        {
            *dist = d0 + d1;
            *point = p;
        }
        
        // There is no possible improvement
        float minrad = 0.02f;
        if (minf(d0, d1) + r <= 0.0f || d0 + d1 - 2 * r > *dist || r < minrad)
        {
            continue;
        }
        
        Bound s0, s1;
        BoundSplit(&s0, &s1, c);
        queue[(queue_start + queue_num + 0) % INTERSECTS_QUEUE_MAX] = s0;
        queue[(queue_start + queue_num + 1) % INTERSECTS_QUEUE_MAX] = s1;
        queue_num += 2;
    }
    
    return false;
}

////////////////////////////

void update_callback(void* args)
{
    ((std::function<void()>*)args)->operator()();
}

int main(void) {
    
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "raylib - sdf collision");

    Shader shader = LoadShader(0, TextFormat("resources/sdf.glsl", 0));
    int resolutionLoc = GetShaderLocation(shader, "u_resolution");
    int shapeType0Loc = GetShaderLocation(shader, "u_shapeType0");
    int shapeType1Loc = GetShaderLocation(shader, "u_shapeType1");
    int shapePos0Loc = GetShaderLocation(shader, "u_shapePos0");
    int shapePos1Loc = GetShaderLocation(shader, "u_shapePos1");
    int shapeRot0Loc = GetShaderLocation(shader, "u_shapeRot0");
    int shapeRot1Loc = GetShaderLocation(shader, "u_shapeRot1");
    int shapeScale0Loc = GetShaderLocation(shader, "u_shapeScale0");
    int shapeScale1Loc = GetShaderLocation(shader, "u_shapeScale1");
    int shapeMargin0Loc = GetShaderLocation(shader, "u_shapeMargin0");
    int shapeMargin1Loc = GetShaderLocation(shader, "u_shapeMargin1");

    shapeType0 = 0;
    shapeType1 = 1;
    shapePos0 = (Vector2){ -0.5f, 0.0f };
    shapePos1 = (Vector2){ +0.5f, 0.0f };
    shapeRot0 = 0.0f;
    shapeRot1 = 0.0f;
    shapeScale0 = 0.5f;
    shapeScale1 = 0.5f;
    shapeMargin0 = 0.0f;
    shapeMargin1 = 0.0f;
    
    int collisionMode = 1;
    bool collisionEdit = false;
    bool breadthFirst = true;

    bool shapeEdit0 = false;
    bool shapeEdit1 = false;
    bool shapeMove0 = false;
    bool shapeMove1 = false;
    
    SetTargetFPS(60);


    auto update_func = [&]()
    {
        float boundExpand = collisionMode == 2 ? 0.25f : 0.0f;

        Bound bb0 = BoundExpand(boundShape0(), (Vector2){ boundExpand, boundExpand });
        Bound bb1 = BoundExpand(boundShape1(), (Vector2){ boundExpand, boundExpand });
        
        Rectangle r0 = BoundScreenRectangle(bb0);
        Rectangle r1 = BoundScreenRectangle(bb1);
        
        if (IsKeyPressed(KEY_S)) 
        {
            TakeScreenshot("screenshot.png"); 
        }
        
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            if (CheckCollisionPointRec(GetMousePosition(), r0))
            {
                shapeMove0 = true;
            }
            else if (CheckCollisionPointRec(GetMousePosition(), r1))
            {
                shapeMove1 = true;
            }
        }
        else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            shapeMove0 = false;
            shapeMove1 = false;
        }
        
        if (shapeMove0)
        {
            Vector2 mouse = GetMouseDelta();
            mouse.y = -mouse.y;
            
            if (IsKeyDown(KEY_LEFT_CONTROL))
            {
                shapeRot0 += (mouse.x + mouse.y) / 200.0f;
            }
            else if (IsKeyDown(KEY_LEFT_SHIFT))
            {
                shapeScale0 *= expf((mouse.x + mouse.y) / 200.0f);
            }
            else
            {
                shapePos0 = Vector2Add(shapePos0, Vector2Scale(mouse, 2.0f / (float)screenHeight));                  
            }
        }
        
        if (shapeMove1)
        {
            Vector2 mouse = GetMouseDelta();
            mouse.y = -mouse.y;

            if (IsKeyDown(KEY_LEFT_CONTROL))
            {
                shapeRot1 += (mouse.x + mouse.y) / 100.0f;
            }
            else if (IsKeyDown(KEY_LEFT_SHIFT))
            {
                shapeScale1 *= expf((mouse.x + mouse.y) / 100.0f);
            }
            else
            {
                shapePos1 = Vector2Add(shapePos1, Vector2Scale(mouse, 2.0f / (float)screenHeight));
            }
        }
        
        Bound intersection = BoundIntersection(bb0, bb1);
        
        // Render

        Vector2 resolution = (Vector2){ screenWidth, screenHeight };
        SetShaderValue(shader, resolutionLoc, &resolution, SHADER_UNIFORM_VEC2);
        SetShaderValue(shader, shapeType0Loc, &shapeType0, SHADER_UNIFORM_INT);
        SetShaderValue(shader, shapeType1Loc, &shapeType1, SHADER_UNIFORM_INT);
        SetShaderValue(shader, shapePos0Loc, &shapePos0, SHADER_UNIFORM_VEC2);
        SetShaderValue(shader, shapePos1Loc, &shapePos1, SHADER_UNIFORM_VEC2);
        SetShaderValue(shader, shapeRot0Loc, &shapeRot0, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, shapeRot1Loc, &shapeRot1, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, shapeScale0Loc, &shapeScale0, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, shapeScale1Loc, &shapeScale1, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, shapeMargin0Loc, &shapeMargin0, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, shapeMargin1Loc, &shapeMargin1, SHADER_UNIFORM_FLOAT);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Enable custom shader
        BeginShaderMode(shader);
        
        // Draw a rectangle that covers the entire screen
        DrawRectangle(0, 0, screenWidth, screenHeight, WHITE);

        EndShaderMode();
        
        DrawBound(bb0, Fade(RED, 0.5f), 1.0f);
        DrawBound(bb1, Fade(BLUE, 0.5f), 1.0f);
        
        if (BoundIsValid(intersection))
        {
            DrawBound(intersection, Fade(GREEN, 0.5f), 1.0f);            
        }
        
        // Intersection Test
        
        if (collisionMode == 0)
        {
            sdSampleNum = 0;
            
            bool intersects = false;
            if (breadthFirst)
            {
                intersects = BoundIsValid(intersection) && sdIntersects2(intersection);
            }
            else
            {
                intersects = BoundIsValid(intersection) && sdIntersects(intersection);
            }

            DrawText(TextFormat("Collided: %s", intersects ? "Yes" : "No"), 20, 20, 20, intersects ? WHITE : GRAY);
            DrawText(TextFormat("Samples: %i", sdSampleNum), 20, 50, 20, intersects ? WHITE : GRAY);
        }
        
        if (collisionMode == 1)
        {
            sdSampleNum = 0;
            
            float depth = 0.0f;
            Vector2 point = (Vector2){ 0.0f, 0.0f };
            if (BoundIsValid(intersection))
            {
                if (breadthFirst)
                {
                    sdPenetration2(&point, &depth, intersection);
                }
                else
                {
                    sdPenetration(&point, &depth, intersection);
                }
            }
            
            bool intersects = depth < 0.0f;
            
            DrawText(TextFormat("Collided: %s", intersects ? "Yes" : "No"), 20, 20, 20, intersects ? WHITE : GRAY);
            DrawText(TextFormat("Samples: %i", sdSampleNum), 20, 50, 20, intersects ? WHITE : GRAY);
            DrawText(TextFormat("Depth: %f", depth), 20, 80, 20, intersects ? WHITE : GRAY);
            
            if (intersects)
            {
                float d0 = sdShape0(point);
                float d1 = sdShape1(point);
                Vector2 n0 = normalShape0(point);
                Vector2 n1 = normalShape1(point);
                
                Vector2 c0 = Vector2Add(point, Vector2Scale(n0, -d0));
                Vector2 c1 = Vector2Add(point, Vector2Scale(n1, -d1));
                
                DrawSurfacePoint(c0, n0, 2.0f, 10.0f, Fade(WHITE, 1.0f));
                DrawSurfacePoint(c1, n1, 2.0f, 10.0f, Fade(WHITE, 1.0f));
                
                DrawSample(point, Fade(WHITE, 1.0f), 3.0f);
            }
        }
        
        if (collisionMode == 2)
        {
            sdSampleNum = 0;
            
            bool valid = BoundIsValid(intersection);
            float distance = 999.0f;
            Vector2 point = (Vector2){ 0.0f, 0.0f };
            bool intersects = false;
            if (valid)
            {
                if (breadthFirst)
                {
                    intersects = sdDistance2(&point, &distance, intersection);
                }
                else
                {
                    intersects = sdDistance(&point, &distance, intersection);    
                }
            }
            
            DrawText(TextFormat("Collided: %s", intersects ? "Yes" : "No"), 20, 20, 20, !intersects ? WHITE : GRAY);
            DrawText(TextFormat("Samples: %i", sdSampleNum), 20, 50, 20, !intersects ? WHITE : GRAY);
            DrawText(TextFormat("Distance: %f", valid ? distance : 0.0f), 20, 80, 20, !intersects ? WHITE : GRAY);
            
            if (!intersects && valid)
            {
                float d0 = sdShape0(point);
                float d1 = sdShape1(point);
                Vector2 n0 = normalShape0(point);
                Vector2 n1 = normalShape1(point);
                
                Vector2 c0 = Vector2Add(point, Vector2Scale(n0, -d0));
                Vector2 c1 = Vector2Add(point, Vector2Scale(n1, -d1));
                
                DrawSurfacePoint(c0, n0, 2.0f, 10.0f, Fade(WHITE, 1.0f));
                DrawSurfacePoint(c1, n1, 2.0f, 10.0f, Fade(WHITE, 1.0f));
                
                DrawSample(point, Fade(WHITE, 1.0f), 3.0f);
            }
        }
        
        // Gui
                
        if (GuiDropdownBox((Rectangle){ 30, 130, 150, 30 }, "Collision;Response;Distance", &collisionMode, collisionEdit))
        {
            collisionEdit = !collisionEdit;
        }
        
        GuiCheckBox((Rectangle){ 190, 130, 30, 30 }, "Breadth First", &breadthFirst);
        
        GuiSliderBar((Rectangle){  80, 270, 70, 30 }, "Margin", TextFormat("%3.1f", shapeMargin0), &shapeMargin0, 0.0f, 0.1f);
        GuiSliderBar((Rectangle){ 240, 270, 70, 30 }, "Margin", TextFormat("%3.1f", shapeMargin1), &shapeMargin1, 0.0f, 0.1f);

        if (GuiDropdownBox((Rectangle){ 30, 320, 150, 30 }, 
            "Heart;Cross;ChamferBox;Circle;Stadium;Triangle;Star;Arc;Horseshoe;Moon;CoolS", &shapeType0, shapeEdit0))
        {
            shapeEdit0 = !shapeEdit0;
        }
        
        if (GuiDropdownBox((Rectangle){ 190, 320, 150, 30 }, 
            "Heart;Cross;ChamferBox;Circle;Stadium;Triangle;Star;Arc;Horseshoe;Moon;CoolS", &shapeType1, shapeEdit1))
        {
            shapeEdit1 = !shapeEdit1;
        }
        
        EndDrawing();
    };
    
#if defined(PLATFORM_WEB)
    std::function<void()> u{update_func};
    emscripten_set_main_loop_arg(update_callback, &u, 0, 1);
#else
    while (!WindowShouldClose())
    {
        update_func();
    }
#endif

    UnloadShader(shader);
    CloseWindow();
    return 0;
}