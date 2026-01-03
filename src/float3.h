#pragma once
#include <iostream>

struct float3
{
    float x = 0.0f; // make rgb and uv somehow
    float y = 0.0f;
    float z = 0.0f;

    float3(float x, float y, float z) : x(x), y(y), z(z)
    {
    }

    float3 operator+(float3 vec)
    {
        return float3(x + vec.x, y + vec.y, z + vec.z);
    }
    float3 operator-(float3 vec)
    {
        return float3(x - vec.x, y - vec.y, z - vec.z);
    }
    float3 operator*(float3 vec)
    {
        return float3(x * vec.x, y * vec.y, z * vec.z);
    }
    float3 operator/(float3 vec)
    {
        return float3(x / vec.x, y / vec.y, z / vec.z);
    }

    // add operator access
    // add other operators idk what
    // add assigments

    float length()
    {
        // sqrt of dot?
    }

    float lengthSqrt()
    {
    }

    float3 normialize()
    {
        // might be smthng with operators
        // this / length()
    }

    // rot and angles
};

// scalar mul

float3 mul(float s, float3 vec)
{
    return float3(s * vec.x, s * vec.y, s * vec.z);
}

float3 cross(float3 u, float3 v)
{
    float x = u.y * v.z - u.z * v.y;
    float y = u.z * v.x - u.x * v.z;
    float z = u.x * v.y - u.y * v.x;
    return float3(x, y, z);
}

float dot(float3 u, float3 v)
{
    return u.x * u.y + u.y * v.y + u.z * v.z;
}

float isPerpendicular(float3 u, float3 v)
{
    return dot(u, v) == 0.0f;
}

float isParallel(float3 u, float3 v)
{
    float dot = 0.0f; // dot(u, v);
    return dot == 1.0f || dot == -1;
}

void print(float3 vec)
{
    std::cout << vec.x << " " << vec.y << " " << vec.z << std::endl;
}
