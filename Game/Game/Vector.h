#pragma once

#include <xmmintrin.h>
#include <string.h>
#include <cmath>

namespace Game {
    //Don't call this function outside.
    template<typename T>
    inline __m128 _pack_vector_to_m128(T t) {
        return _pack_vector_to_m128<float>(static_cast<float>(t));
    }

    template<>
    inline __m128 _pack_vector_to_m128(float num) {
        __m128 temp = _mm_load_ss(&num);
        return _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(0, 0, 0, 0));
    }


    //Don't invoke this function external.Call the dot function 
    //to get two vectors' dot product 
    float _packed_dot(__m128 v1, __m128 v2);

    struct Vector2 {
        union {
            float raw[2];
            struct {
                float x, y;
            };
        };
        Vector2() :x(0), y(0) {}
        Vector2(float x, float y) :x(x), y(y) {}
        Vector2(float* v) :x(v[0]), y(v[1]) { }

        //don't call this function to construct the vector.
        Vector2(__m128 m);

        inline float& operator[](int index) { return raw[index]; }
        inline const float& operator[](int index) const { return raw[index]; }
    };

    template<>
    inline __m128 _pack_vector_to_m128(Vector2 v) {
        float buffer[4] = { 0,0,0,0 };
        memcpy(buffer, v.raw, sizeof(float) * 2);
        return _mm_load_ps(buffer);
    }

    struct Vector3 {
        union {
            float raw[3];
            struct {
                float x, y, z;
            };
        };

        Vector3() :x(0), y(0), z(0) {}
        Vector3(float x, float y, float z) :x(x), y(y), z(z) {}
        Vector3(float* f) :x(f[0]), y(f[1]), z(f[2]) {}

        //don't call this function to construct the vector.
        Vector3(__m128 m);

        inline float& operator[](int index) { return raw[index]; }
        inline const float& operator[](int index) const { return raw[index]; }
    };

    template<>
    inline __m128 _pack_vector_to_m128(Vector3 v) {
        float buffer[4] = { 0,0,0,0 };
        memcpy(buffer, v.raw, sizeof(float) * 3);
        return _mm_load_ps(buffer);
    }

    struct Vector4 {
        union {
            float raw[4];
            struct {
                float x, y, z, w;
            };
        };
        Vector4() :x(0), y(0), z(0), w(0) {}
        Vector4(float x, float y, float z, float w) :x(x), y(y), z(z), w(w) {}
        Vector4(float* f) :x(f[0]), y(f[1]), z(f[2]), w(f[3]) {}

        //don't call this function to construct the vector.
        Vector4(__m128 m);

        inline float& operator[](int index) { return raw[index]; }
        inline const float& operator[](int index) const { return raw[index]; }
    };

    template<>
    inline __m128 _pack_vector_to_m128(Vector4 v) {
        return _mm_load_ps(v.raw);
    }

    template<typename Vec>
    inline float dot(const Vec& lhs, const Vec& rhs) {
        return _packed_dot(
            _pack_vector_to_m128(lhs),
            _pack_vector_to_m128(rhs)
        );
    }

    template<typename Vec, typename Vec2>
    inline Vec operator*(const Vec& v1, const Vec2& v2) {
        return  Vec(_mm_mul_ps(
            _pack_vector_to_m128(v1),
            _pack_vector_to_m128(v2)
        )
        );
    }

    template<typename Vec, typename Vec2>
    inline Vec operator+(const Vec& v1, const Vec2& v2) {
        return  Vec(_mm_add_ps(
            _pack_vector_to_m128(v1),
            _pack_vector_to_m128(v2)
        )
        );
    }

    template<typename Vec, typename Vec2>
    inline Vec operator-(const Vec& v1, const Vec2& v2) {
        return  Vec(_mm_sub_ps(
            _pack_vector_to_m128(v1),
            _pack_vector_to_m128(v2)
        )
        );
    }

    template<typename Vec>
    inline Vec operator/(const Vec& v1, float v2) {
        return  Vec(_mm_div_ps(
            _pack_vector_to_m128(v1),
            _pack_vector_to_m128(v2)
        )
        );
    }

    Vector3 cross(const Vector3& lhs, const Vector3& rhs);

    template<typename Vec>
    inline float length(const Vec& v) {
        return sqrt(dot(v,v));
    }

    template<typename Vec>
    inline Vec normalize(const Vec& v) {
        return v / length(v);
    }

    template<typename Vec>
    inline Vec lerp(Vec a,Vec b,float f) {
        return a * (1 - f) + b * f;
    }
}