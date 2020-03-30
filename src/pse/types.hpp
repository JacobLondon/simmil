#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#undef main

namespace pse {

template<typename T>
struct vec2 {
    T x, y;
    vec2()  : x{}, y{} {}
    vec2(T x, T y) : x{x}, y{y} {}
    void add(T x, T y)
    {
        this->x += x;
        this->y += y;
    }

};

template<typename T>
struct vec3 {
    T x, y, z;
    vec3() : x{0}, y{0}, z{0} {}
    vec3(T x, T y, T z) : x{x}, y{y}, z{z} {}
};

template<typename T>
struct tri2 {
    vec2<T> a, b, c;
    tri2()
         : a{}, b{}, c{} {}
    tri2(vec2<T> a, vec2<T> b, vec2<T> c)
         : a{ a.x, a.y }, b{ b.x, b.y }, c{ c.x, c.y } {}
    tri2(int ax, int ay, int bx, int by, int cx, int cy)
         : a{ ax, ay }, b{ bx, by }, c{ cx, cy } {}
};

template<typename T>
bool point_in_triangle(vec2<T>& s, tri2<T>& t)
{
    vec2<T>& a = t.a;
    vec2<T>& b = t.b;
    vec2<T>& c = t.c;

    int as_x = s.x - a.x;
    int as_y = s.y - a.y;

    bool s_ab = (b.x - a.x) * as_y - (b.y - a.y) * as_x > 0;

    if (((c.x - a.x) * as_y - (c.y - a.y) * as_x > 0) == s_ab) return false;

    if (((c.x - b.x) * (s.y - b.y) - (c.y - b.y) * (s.x - b.x) > 0) != s_ab) return false;

    return true;
}

using ivec2 = vec2<int>;
using ivec3 = vec3<int>;

} // pse