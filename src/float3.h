class float3
{
    float x = 0; //make rgb and uv somehow
    float y = 0;
    float z = 0;

    float3(float x, float y, float z) 
    : this.x(x), this.y(y), this.z(z)
    {

    }

    float3(glm::vec3 vec)
    : this.x(vec.x), this.y(vec.y), this.z(vec.z)
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
    float3 operator%(float3 vec)
    {
        return float3(x % vec.x, y % vec.y, z % vec.z);
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

    float3 normialize()
    {
        //might be smthng with operators
        //this / length()
    }

    //rot and angles
}

//scalar mul

float mul( float s, float3 vec)
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
    float dot = dot(u, v);
    return dot == 1.0f || dot == -1;
}


matrix4x4 orthographicProjection(float left, float right, float bottom, float top, float near, float far)
{
    matrix4x4 mat;
    mat[0] = {2 / (right - left), 0, 0, -(right + left) / (right - left)};
    mat[1] = {0, 2 / (top - bottom), 0, -(top + bottom) / (top - bottom)};
    mat[2] = {0, 0, 1 / (far - near), -(near / (far - near))};
    mat[3] = {0, 0, 0, 1};
    return mat;
}
