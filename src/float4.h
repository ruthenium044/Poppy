#pragma once
#include "float3.h"

struct float4
{
    float x = 0.0f; //make rgb and uv somehow
    float y = 0.0f;
    float z = 0.0f;
	float w = 0.0f;

    float4(float x, float y, float z, float w)
		: x(x), y(y), z(z), w(w)
    {
    }

    float4(float x, float y, float z)
        : x(x), y(y), z(z), w(1.0f)
    {
	}

    float4(float3 vec)
        : x(vec.x), y(vec.y), z(vec.z), w(1.0f)
    {
	}

    float4 operator+(float4 vec)
    {
        return float4(x + vec.x, y + vec.y, z + vec.z, w + vec.w);
    }
    float4 operator-(float4 vec)
    {
        return float4(x - vec.x, y - vec.y, z - vec.z, w - vec.w);
    }
    float4 operator*(float4 vec)
    {
        return float4(x * vec.x, y * vec.y, z * vec.z, w * vec.w);
    }
    float4 operator/(float4 vec)
    {
        return float4(x / vec.x, y / vec.y, z / vec.z, w / vec.w);
    }

    //add operator access
    //add other operators idk what
    //add assigments

    float length()
    {
        //sqrt of dot?
    }

    float lengthSqrt()
    {

    }

    float4 normialize()
    {
        //might be smthng with operators
        //this / length()
    }

    //rot and angles
};