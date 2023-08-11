#include <math.h>
#include <stdio.h>

float min(float a, float b)
{
    return (a < b) ? a : b;
}

float max(float a, float b)
{
    return (a > b) ? a : b;
}

float clamp(float x, float a, float b)
{
    return max(min(x, a), b);
}

typedef struct vec3
{
    float x, y, z;
} vec3;

void print_vec3(vec3 *v)
{
    printf("{x = %.3f, y = %.3f, z = %.3f}", v->x, v->y, v->z);
}

typedef struct vec4
{
    float x, y, z, w;
} vec4;

void print_vec4(vec4 *v)
{
    printf("{x = %.3f, y = %.3f, z = %.3f, w = %.3f}", v->x, v->y, v->z, v->w);
}

vec3 scalar_mul(float x, vec3 v)
{
    v.x *= x;
    v.y *= x;
    v.z *= x;
    return v;
}

vec3 divf(vec3 a, float b)
{
    a.x /= b;
    a.y /= b;
    a.z /= b;
    return a;
}

vec3 mul(vec3 a, vec3 b)
{
    a.x *= b.x;
    a.y *= b.y;
    a.z *= b.z;
    return a;
}

vec3 sub(vec3 a, vec3 b)
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    return a;
}

vec3 add(vec3 a, vec3 b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}

const vec3 zero = {0, 0, 0};

vec3 neg(vec3 x)
{
    return sub(zero, x);
}

vec3 cross(vec3 a, vec3 b)
{
    vec3 out;
    out.x = a.y * b.z - a.z * b.y;
    out.y = a.z * b.x - a.x * b.z;
    out.z = a.x * b.y - a.y * b.x;
    return out;
}

float dot(vec3 a, vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float norm_sq(vec3 a)
{
    return dot(a, a);
}

float norm(vec3 a)
{
    return sqrt(norm_sq(a));
}

vec3 unit(vec3 x)
{
    return divf(x, norm(x));
}

typedef struct mat4
{
    float x[4][4];
} mat4;

float ix(mat4 a, size_t i, size_t j)
{
    return a.x[i][j];
}

vec4 vec4_new(float x, float y, float z, float w)
{
    vec4 out = {x, y, z, w};
    return out;
}

vec4 row(mat4 a, size_t i)
{
    switch (i) {
        case 0:
            return vec4_new(ix(a, 0, 0), ix(a, 0, 1), ix(a, 0, 2), ix(a, 0, 3));
        case 1:
            return vec4_new(ix(a, 1, 0), ix(a, 1, 1), ix(a, 1, 2), ix(a, 1, 3));
        case 2:
            return vec4_new(ix(a, 2, 0), ix(a, 2, 1), ix(a, 2, 2), ix(a, 2, 3));
        case 3:
            return vec4_new(ix(a, 3, 0), ix(a, 3, 1), ix(a, 3, 2), ix(a, 3, 3));
        default:
            printf("error: bad index: %zu\n", i);
            exit(1);
    }
}

vec4 col(mat4 a, size_t i)
{
    switch (i) {
        case 0:
            return vec4_new(ix(a, 0, 0), ix(a, 1, 0), ix(a, 2, 0), ix(a, 3, 0));
        case 1:
            return vec4_new(ix(a, 0, 1), ix(a, 1, 1), ix(a, 2, 1), ix(a, 3, 1));
        case 2:
            return vec4_new(ix(a, 0, 2), ix(a, 1, 2), ix(a, 2, 2), ix(a, 3, 2));
        case 3:
            return vec4_new(ix(a, 0, 3), ix(a, 1, 3), ix(a, 2, 3), ix(a, 3, 3));
        default:
            printf("error: bad index: %zu\n", i);
            exit(1);
    }
}

mat4 matmul(mat4 a, mat4 b)
{
    printf("error: not implemented\n");
    exit(1);
}
