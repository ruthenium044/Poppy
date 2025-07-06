#pragma once

class Allocato
{
public:
	virtual ~Allocato() = default;
	virtual void* alloc(size_t size) = 0;
	virtual void free(void* ptr) = 0;

	void* realloc(void* ptr, size_t newSize, size_t oldSize)
	{
		if (ptr == nullptr && newSize == 0)
		{
			return nullptr;
		}
		if (ptr == nullptr)
		{
			return alloc(newSize);
		}
		if (newSize == 0)
		{
			free(ptr);
			return nullptr;
		}
		unsigned char* dst = (unsigned char*)alloc(newSize);
		unsigned char* src = (unsigned char*)ptr;
		size_t copySize = (oldSize < newSize) ? oldSize : newSize;
		for (size_t i = 0; i < copySize; ++i)
		{
			dst[i] = src[i];
		}
		free(ptr);
		return dst;
	}
};

class HeapAllocato : public Allocato
{
	virtual void* alloc(size_t size) final
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
		if (capacity <= 0)
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

	Listo(const Listo& other) //copy constructor
		: allocato(other.allocato), size(other.size), capacity(other.capacity)
	{
		data = (int*)other.allocato->alloc(other.capacity * sizeof(int));
	}

	int* get(int index) 
	{
		SDL_assert(index >= 0 && "Index out of bounds in Listo::get");
		return &data[index];
	}

	int& operator[](int index) { return *get(index); }

	int getFront() const { return data[0]; }
	int getBack() const { return data[size]; }
	int getSize() const { return size; }
	bool isEmpty() const { return size == 0; }

	int* begin() { return data; }
	int* end() { return data + size; }

	void maybeResize(int newSize)
	{
		if (newSize > capacity)
		{
			capacity *= 2;
			allocato->realloc(data, capacity, size);
		}
	}

	void set(int index, int value)
	{
		SDL_assert(index < size && "Index out of bounds in Listo::set");
		SDL_assert(size > 0 && "Listo::popFront called on empty list");
		
		data[index] = value;
	}

	void pushFront(int value)
	{
		size_t nextSize = size + 1;
		maybeResize(nextSize);
		memmove(data + 1, data, size * sizeof(*data));
		data[0] = value;
		size = nextSize;
	}

	void pushBack(int value)
	{
		size_t nextSize = size + 1;
		maybeResize(nextSize);
		
		data[size] = value;
		size = nextSize;
	}

	int popFront()
	{
		SDL_assert(size > 0 && "Listo::popFront called on empty list");
		int value = data[0];
		--size;
		memmove(data, data + 1, size * sizeof(*data));
		return value;
	}

	int popBack()
	{
		SDL_assert(size > 0 && "Listo::popFront called on empty list");
		int value = data[size];
		--size;
		return value;
	}

	void remove(int index)
	{
		SDL_assert(size > 0 && "Listo::popFront called on empty list");
		SDL_assert(index < size && "Index out of bounds in Listo::remove");

		if (index == size - 1)
		{
			popBack();
		}
		else
		{
			size_t offset = size - index;
			size_t srcIndex = index + 1;
			size_t dstIndex = index;
			memmove(data + dstIndex, data + srcIndex, offset * sizeof(*data));
			size--;
		}
	}

	void clear()
	{
		SDL_assert(size > 0 && "Listo::popFront called on empty list");
		size = 0;
	}

	void insert(int index, int value)
	{
		SDL_assert(size > 0 && "Listo::popFront called on empty list");
		SDL_assert(index < size && "Index out of bounds in Listo::insert");

		if (index == size)
		{
			pushBack(value);
		}
		else
		{
			size_t offset = size - index;
			size_t srcIndex = index;
			size_t dstIndex = index + 1;
			memmove(data + dstIndex, data + srcIndex, offset * sizeof(*data));

			data[index] = value;
			size++;
		}

	}
};
