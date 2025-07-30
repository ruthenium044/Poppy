#pragma once

struct mat4x4
{
	union
	{
		float elements[16]{};
		float m[4][4]{};
		struct {
			float m00 = 0.0f, m01 = 0.0f, m02 = 0.0f, m03 = 0.0f;
			float m10 = 0.0f, m11 = 0.0f, m12 = 0.0f, m13 = 0.0f;
			float m20 = 0.0f, m21 = 0.0f, m22 = 0.0f, m23 = 0.0f;
			float m30 = 0.0f, m31 = 0.0f, m32 = 0.0f, m33 = 0.0f;
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

mat4x4 operator+(const float4& vec)
{

}

//not commutative
mat4x4 operator*( mat4x4 mat, float4 vec )
{
	
}

mat4x4 translate(mat4x4 mat, float3 vec)
{
	//mat4x4 translation = mat4x4(
   	// 1.0, 0.0, 0.0, vec.x,
   	// 0.0, 1.0, 0.0, vec.y,
   	// 0.0, 0.0, 1.0, vec.z,
   	// 0.0, 0.0, 0.0, 1.0);
	//return translation * mat;

	//which is same as:
	
	mat.m03 += vec.x;
	mat.m13 += vec.y;
	mat.m23 += vec.z;
	return mat;
}

mat4x4 scale(mat4x4 mat, float3 vec)
{
	//mat4x4 scale = mat4x4(
   	// vec.x, 0.0, 0.0, 0.0,
   	// 0.0, vec.y, 0.0, 0.0,
   	// 0.0, 0.0, vec.z, 0.0,
   	// 0.0, 0.0, 0.0, 1.0);
	//return scale * mat;

	//which is same as:
	mat.m00 *= vec.x;
	mat.m11 *= vec.y;
	mat.m22 *= vec.z;
	return mat;
}

//
//mat4x4 orthographicProjection(float left, float right, float bottom, float top, float near, float far)
//{
//    matrix4x4 mat;
//    mat[0] = {2 / (right - left), 0, 0, -(right + left) / (right - left)};
//    mat[1] = {0, 2 / (top - bottom), 0, -(top + bottom) / (top - bottom)};
//    mat[2] = {0, 0, 1 / (far - near), -(near / (far - near))};
//    mat[3] = {0, 0, 0, 1};
//    return mat;
//}
