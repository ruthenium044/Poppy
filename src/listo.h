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
    
    int get(int index)
    {
        if(size < index)
        {
            //fuck you
        }
        return data[index];
    }

    int& operator[](int index) { return get(index) }

    int getFront() { return data[0]; }
    int getBack() { return data[size]; }
    int getSize() { return size; }
    bool isEmpty() { return size < 0; }

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

    void pushFront(int value)
    {
        if(size == capacity)
        {
            resize();
        }
        memmove(1, data + size, size);
        data[0] = value
        size++
    }

    void pushBack(int value)
    {
        if(size == capacity)
        {
            resize();
        }
        //will this increase size?
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

        memmove(data + at + 1, data + at, size - at);
        data[at] = value
        size++
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
