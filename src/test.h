class Listo
{
    Listo(int size)
    {
        this.size = size;
        //what capacity?
        this.capacity = size + 1; //idk
    }
    ~Listo()
    {
        //idk delete shit
    }
    //[] operator pls
    int get(int index)
    {
        if(size < index)
        {
            //fuck you
        }
        return data[index];
    }

    int getFront()
    {
        return data[0];
    }

    int getBack()
    {
        return data[size];
    }

    int getSize()
    {
        return size;
    }

    bool isEmpty()
    {
        return size < 0;
    }

    void resize()
    {
        //this by user or auto?
        //how to?
        if(size < index)
        {
            //give new size or what capacity or what?
        }
    }

    void set(int value, int index) //actually which order is more clear?
    {
        if(size < index)
        {
            resize();
        }
        data[index] = value;
    }

    void push()
    {
        //whats the dif?
        //what params
        
    }

    void pushFront(int value)
    {
        if(size == capacity)
        {
            resize();
        }
        //more like dafuq im not iterating over all elements
    }

    void pushBack(int value)
    {
        if(size == capacity)
        {
            resize();
        }
        //increase size?
        data[size++] = value;
    }

     void popFront()
    {
        if(size < 0)
        {
            //again i said no lol
        }

        --size;
        //yeah not gonna iterate
    }

    void popBack()
    {
        if(size < 0)
        {
            //again i said no lol
        }

        --size;
        //just change size or what?
        //how to get rid of shit lol
    }

    void insert(int value, int index)
    {
        if(size < index)
        {
            //nope
        }

        if(index == 0)
        {
            pushFront(value);
        }
        if(index == size)
        {
            pushBack(value);
        }
        //overiterating AGAIIN?!
    }

    void print()
    {
        //todo for debug?
    }

private:

    int size = 0;
    int capacity = 0;

    int* data;
}


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
