/*
 * Computer Graphics course
 * Dept Computing Science, Umea University
 * Stefan Johansson, stefanj@cs.umu.se
 */
#pragma once

#include <iostream>
#include <fstream>
#include <memory>
#include <cmath>
#include <vector>

#define BUFFER_OFFSET(i) (reinterpret_cast<char*>(0 + (i)))

struct Vec2 {
    float x, y;
    Vec2() : x(0.0f), y(0.0f) {}
    Vec2(float X, float Y) : x(X), y(Y) {}
};

struct Vec3 {
    float x, y, z;
    Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vec3(float X, float Y, float Z) : x(X), y(Y), z(Z) {}
};

struct Vec4 {
    float x, y, z, w;
    Vec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    Vec4(float X, float Y, float Z, float W) : x(X), y(Y), z(Z), w(W) {}
};

struct Vertex {
    Vec3 position;
    Vec3 normal;
    Vec2 texCoord;

    Vertex() = default;
    Vertex(const Vec3& pos, const Vec3& norm) : position(pos), normal(norm), texCoord(0.0f, 0.0f) {}
    Vertex(const Vec3& pos, const Vec3& norm, const Vec2& uv) : position(pos), normal(norm), texCoord(uv) {}
};


inline float dot(const Vec3& a, const Vec3& b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

inline Vec3 cross(const Vec3& a, const Vec3& b) {
    return Vec3(
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x
    );
}

inline float length(const Vec3& v) {
    return std::sqrt(dot(v, v));
}

inline Vec3 normalize(const Vec3& v) {
    float len = length(v);
    if (len <= 0.0f) return Vec3(0.0f, 0.0f, 0.0f);
    return Vec3(v.x/len, v.y/len, v.z/len);
}

inline Vec3 add(const Vec3& a, const Vec3& b) { return Vec3(a.x+b.x, a.y+b.y, a.z+b.z); }
inline Vec3 sub(const Vec3& a, const Vec3& b) { return Vec3(a.x-b.x, a.y-b.y, a.z-b.z); }
inline Vec3 mul(const Vec3& v, float s) { return Vec3(v.x*s, v.y*s, v.z*s); }

inline Vec3 xyz(const Vec4& v) { return Vec3(v.x, v.y, v.z); }

// Debug macros
#define DEBUG( msg ) \
    std::cerr << "DBG " << __FILE__ << "(" << __LINE__ << "): " << msg << std::endl

#define DEBUGVAR( var )\
    std::cerr << "DBG " << __FILE__ << "(" << __LINE__ << "): " \
    << #var << " = " << (var) << std::endl