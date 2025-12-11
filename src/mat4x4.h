#pragma once
#include "float3.h"
#include "float4.h"

struct mat4x4
{
	union
	{
		float elements[16];
		float m[4][4];
		struct {
			float m00, m01, m02, m03;
			float m10, m11, m12, m13;
			float m20, m21, m22, m23;
			float m30, m31, m32, m33;
		};
	};

	mat4x4() {}

	mat4x4(float value)
		: m00(value), m01(0.0f), m02(0.0f), m03(0.0f),
		m10(0.0f), m11(value), m12(0.0f), m13(0.0f),
		m20(0.0f), m21(0.0f), m22(value), m23(0.0f),
		m30(0.0f), m31(0.0f), m32(0.0f), m33(value)
	{

	}

	mat4x4(float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33)
		: m00(m00), m01(m01), m02(m02), m03(m03),
		m10(m10), m11(m11), m12(m12), m13(m13),
		m20(m20), m21(m21), m22(m22), m23(m23),
		m30(m30), m31(m31), m32(m32), m33(m33)
	{

	}

};

//mat4x4 operator+(const float4& vec)
//{
//	
//}

//not commutative
float4 operator*(mat4x4 mat, float4 vec)
{
	float4 result = {
	mat.m00 * vec.x + mat.m01 * vec.y + mat.m02 * vec.z + mat.m03 * vec.w,
	mat.m10 * vec.x + mat.m11 * vec.y + mat.m12 * vec.z + mat.m13 * vec.w,
	mat.m20 * vec.x + mat.m21 * vec.y + mat.m22 * vec.z + mat.m23 * vec.w,
	mat.m30 * vec.x + mat.m31 * vec.y + mat.m32 * vec.z + mat.m33 * vec.w };
	return result;
}


//TODO fix this
//mat4x4 operator*( mat4x4 matA, mat4x4 matB )
//{
//	mat4x4 result = mat4x4(0.0f);
//	for (int i = 0; i < 4; ++i) 
//	{
//        result[i] = matA * matB[i];
//    }
//	return result;
//}

mat4x4 translate(mat4x4 mat, float3 vec)
{
	mat4x4 result = mat4x4(1.0f);
	result.m30 = mat.m30 + vec.x;
	result.m31 = mat.m31 + vec.y;
	result.m32 = mat.m32 + vec.z;
	return result;
}

//TODO fix this
// 
//float3 translate(float3 vec, float3 translate) //maybe nope
//{
//	//mat4x4 translation = mat4x4(
//   	// 1.0, 0.0, 0.0, vec.x,
//   	// 0.0, 1.0, 0.0, vec.y,
//   	// 0.0, 0.0, 1.0, vec.z,
//   	// 0.0, 0.0, 0.0, 1.0);
//	//return translation * mat;
//
//	//which is same as:
//
//	mat4x4 result = mat4x4(1.0f);
//	vec.x += vec.x;
//	result.m13 = mat.m13 + vec.y;
//	result.m23 = mat.m23 + vec.z;
//	return result;
//}

mat4x4 scale(mat4x4 mat, float3 vec)
{
	mat.m00 *= vec.x;
	mat.m11 *= vec.y;
	mat.m22 *= vec.z;
	return mat;
}

mat4x4 rotationX(mat4x4 mat, float angle)
{
	float sinAngle = sin(angle);
	float cosAngle = cos(angle);

	float m01 = mat.m01, m11 = mat.m11, m21 = mat.m21, m31 = mat.m31;
	float m02 = mat.m02, m12 = mat.m12, m22 = mat.m22, m32 = mat.m32;

	mat.m01 = m01 * cosAngle + m02 * sinAngle;
	mat.m11 = m11 * cosAngle + m12 * sinAngle;
	mat.m21 = m21 * cosAngle + m22 * sinAngle;
	mat.m31 = m31 * cosAngle + m32 * sinAngle;

	mat.m02 = - m01 * sinAngle + m02 * cosAngle;
	mat.m12 = - m11 * sinAngle + m12 * cosAngle;
	mat.m22 = - m21 * sinAngle + m22 * cosAngle;
	mat.m32 = - m31 * sinAngle + m32 * cosAngle;
	
	return mat;
}

mat4x4 rotationY(mat4x4 mat, float angle)
{
	float sinAngle = sin(angle);
	float cosAngle = cos(angle);

	float m00 = mat.m00, m10 = mat.m10, m20 = mat.m20, m30 = mat.m30;
	float m02 = mat.m02, m12 = mat.m12, m22 = mat.m22, m32 = mat.m32;

	mat.m00 = m00 * cosAngle + m02 * sinAngle;
	mat.m10 = m10 * cosAngle + m12 * sinAngle;
	mat.m20 = m20 * cosAngle + m22 * sinAngle;
	mat.m30 = m30 * cosAngle + m32 * sinAngle;

	mat.m02 = -m00 * sinAngle + m02 * cosAngle;
	mat.m12 = -m10 * sinAngle + m12 * cosAngle;
	mat.m22 = -m20 * sinAngle + m22 * cosAngle;
	mat.m32 = -m30 * sinAngle + m32 * cosAngle;

	return mat;
}

mat4x4 rotationZ(mat4x4 mat, float angle)
{
	float sinAngle = sin(angle);
	float cosAngle = cos(angle);

	float m00 = mat.m00, m10 = mat.m10, m20 = mat.m20, m30 = mat.m30;
	float m01 = mat.m01, m11 = mat.m11, m21 = mat.m21, m31 = mat.m31;

	mat.m00 = m00 * cosAngle + m01 * sinAngle;
	mat.m10 = m10 * cosAngle + m11 * sinAngle;
	mat.m20 = m20 * cosAngle + m21 * sinAngle;
	mat.m30 = m30 * cosAngle + m31 * sinAngle;

	mat.m01 = -m00 * sinAngle + m01 * cosAngle;
	mat.m11 = -m10 * sinAngle + m11 * cosAngle;
	mat.m21 = -m20 * sinAngle + m21 * cosAngle;
	mat.m31 = -m30 * sinAngle + m31 * cosAngle;

	return mat;
}

//mat4x4 orthographicProjection(float left, float right, float bottom, float top, float near, float far)
//{
//    matrix4x4 mat;
//    mat[0] = {2 / (right - left), 0, 0, -(right + left) / (right - left)};
//    mat[1] = {0, 2 / (top - bottom), 0, -(top + bottom) / (top - bottom)};
//    mat[2] = {0, 0, 1 / (far - near), -(near / (far - near))};
//    mat[3] = {0, 0, 0, 1};
//    return mat;
//}

void print(mat4x4 mat)
{
	std::cout << mat.m00 << " " << mat.m01 << " " << mat.m02 << " " << mat.m03 << std::endl;
	std::cout << mat.m10 << " " << mat.m11 << " " << mat.m12 << " " << mat.m13 << std::endl;
	std::cout << mat.m20 << " " << mat.m21 << " " << mat.m22 << " " << mat.m23 << std::endl;
	std::cout << mat.m30 << " " << mat.m31 << " " << mat.m32 << " " << mat.m33 << std::endl;
}