class Allocato
{
public:
	virtual void* alloc(size_t size) = 0;
	virtual void free(void* ptr) = 0;
};

class HeapAllocato : public Allocato
{
	virtual void* alloc(size_t size)
	{
		return ::operator new(size);
	}

	virtual void free(void* ptr) final 
	{
		return ::operator delete(ptr);
	}
};

class Listo
{
	Allocato* allocato = nullptr;
	int* data = nullptr;

	size_t size = 0;
	size_t capacity = 0;
public:

	Listo(Allocato* allocato, size_t capacity)
		: allocato(allocato), capacity(capacity)
	{
		if(capacity <= 0)
		{
			capacity = 1;
		}
		data = (int*)allocato->alloc(capacity * sizeof(int));
	}

	~Listo()
	{
		const size_t invalid = (size_t)0xbeefbeefbeefbeef;
		allocato->free(data);
		allocato = (Allocato*)invalid;
		data = (int*)invalid;
		size = 0;
		capacity = 0;
	}

	Listo(const Listo& other)
		: allocato(other.allocato), capacity(other.capacity), size(other.size)
	{
		data = (int*)other.allocato->alloc(other.capacity * sizeof(int));
		

	}
	
	int get(int index)
	{
		if(size < index)
		{
			//fuck you
		}
		return data[index];
	}

	//int& operator[](int index) { return get(index); };

	//int getFront() { return data[0]; }
	//int getBack() { return data[size]; }
	//int getSize() { return size; }
	//bool isEmpty() { return size < 0; }

	//void resize()
	//{
	//	//this by user or auto?
	//	//how to?
	//	if(size < index)
	//	{
	//		//give new size or what capacity or what?
	//	}
	//}

	//void set(int value, int index) //actually which order is more clear?
	//{
	//	if(size < index)
	//	{
	//		resize();
	//	}
	//	data[index] = value;
	//}

	//void pushFront(int value)
	//{
	//	if(size == capacity)
	//	{
	//		resize();
	//	}
	//	memmove(1, data + size, size);
	//	data[0] = value
	//	size++
	//}

	//void pushBack(int value)
	//{
	//	if(size == capacity)
	//	{
	//		resize();
	//	}
	//	//will this increase size?
	//	data[size++] = value;
	//}

	// void popFront()
	//{
	//	if(size < 0)
	//	{
	//		//again i said no lol
	//	}

	//	--size;
	//	//yeah not gonna iterate
	//}

	//void popBack()
	//{
	//	if(size < 0)
	//	{
	//		//again i said no lol
	//	}

	//	--size;
	//	//just change size or what?
	//	//how to get rid of shit lol
	//}

	//void insert(int value, int index)
	//{
	//	if(size < index)
	//	{
	//		//nope
	//	}
	//	if(index == 0)
	//	{
	//		pushFront(value);
	//	}
	//	if(index == size)
	//	{
	//		pushBack(value);
	//	}

	//	memmove(data + at + 1, data + at, size - at);
	//	data[at] = value
	//	size++
	//}

	void print()
	{
		//todo for debug?
	}

};
