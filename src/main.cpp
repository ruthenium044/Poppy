#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <glad/glad.h>
#include <iostream>
#include <filesystem>
#include "SDL3/SDL_stdinc.h"
#include "shader.h"
#include "float3.h"
#include "float4.h"
#include "mat4x4.h"

extern "C" {
	

	//move to .cpp
	enum xxx_shader_type
	{
		XXX_SHADER_TYPE_VERTEX = 0,
		XXX_SHADER_TYPE_FRAGMENT = 1,
	};
	//resources
	struct xxx_buffer
	{

	};

	struct xxx_shader
	{

	};

	struct xxx_program
	{
		unsigned int id;
	};

	struct xxx_graphicsPipeline
	{
		xxx_program program;
		
	};

	struct xxx_renderer
	{
		SDL_GLContext glContext;
		xxx_graphicsPipeline rectPipeline;
		xxx_graphicsPipeline circlePipeline;
		xxx_graphicsPipeline spritePipeline;
	};

	//apis
	//these are only in .h ecverything else in cpp
	struct xxx_bufferApi
	{

		void(*destroy)(struct xxx_bufferApi*);
	};

	struct xxx_shaderApi
	{
		struct xxx_shader* (*create)(xxx_shader_type type, char* source, size_t size);
		void(*destroy)(struct xxx_shader*); //in this one delete program
	};

	struct xxx_programApi
	{
		struct xxx_program* (*create)(void);
		void (*destroy)(struct xxx_program* program);
	};

	struct xxx_api
	{
		struct xxx_renderer* (*create)(void);
		void(*destroy)(struct xxx_renderer*);
		struct xxx_shaderApi* shader;
		struct xxx_programApi* program;
	};

	struct xxx_api* xxx_load();
}

struct ConstantTexture
{
	GLuint value;
};

struct ConstantTextureLookupItem
{
	const char* key;
	ConstantTexture texture;
};

struct ConstantTextureLookup
{
	ConstantTextureLookupItem* items;
	size_t capacity;
};

static unsigned long long ConstantTextureLookupHash(const char* str)
{
	unsigned long long hash = 5381;
	while (*str)
	{
		char c = *str++;
		hash = ((hash << 5) + hash) + (unsigned char)c;
	}

	return hash;
}

ConstantTextureLookup ConstantTextureLookupCreate(size_t capacity)
{
	ConstantTextureLookup lookup;
	lookup.items = (ConstantTextureLookupItem*)malloc(sizeof(*lookup.items) * capacity);
	lookup.capacity = capacity;
	memset(lookup.items, 0, sizeof(*lookup.items) * capacity);
	return lookup;
}

void ConstantTextureLookupAdd(ConstantTextureLookup* lookup, const char* key, ConstantTexture* texture)
{
	unsigned long long slot = ConstantTextureLookupHash(key) % lookup->capacity;
	for (size_t index = 0; index < lookup->capacity; index++)
	{
		size_t at = (slot + index) % lookup->capacity;
		ConstantTextureLookupItem* item = &lookup->items[at];
		if (item->key == nullptr)
		{
			item->key = key;
			item->texture.value = texture->value;
			return;
		}
		int result = strcmp(item->key, key);
		if (result == 0)
		{
			return;
		}
		// Collision, result != 0 -> different key stored already, continue
	}
}

ConstantTexture* ConstantTextureLookupGet(ConstantTextureLookup* lookup, const char* key)
{
	unsigned long long slot = ConstantTextureLookupHash(key) % lookup->capacity;
	for (size_t index = 0; index < lookup->capacity; index++)
	{
		size_t at = (slot + index) % lookup->capacity;
		ConstantTextureLookupItem* item = &lookup->items[at];
		if (item->key == nullptr)
		{
			return nullptr;
		}
		int result = strcmp(item->key, key);
		if (result == 0)
		{
			return &item->texture;
		}
	}
	// Increase load factor pls
	return nullptr;
}

ConstantTextureLookup textureLookup = ConstantTextureLookupCreate(128);

void GlobalTextureAdd(const char* key, ConstantTexture* texture)
{
	ConstantTextureLookupAdd(&textureLookup, key, texture);
}

ConstantTexture* GlobalTextureGet(const char* key)
{
	return ConstantTextureLookupGet(&textureLookup, key);
}

static void GetSimple2dShader()
{
	//attributes: position, color, texture coords
	float vertices[] = {
		// positions          // colors           // texture coords
		 0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
		 0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
		-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
	};
	
	unsigned int indices[] =  // note that we start from 0!
	{
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};

	//GL_STREAM_DRAW: the data is set only once and used by the GPU at most a few times.
	//GL_STATIC_DRAW : the data is set only once and used many times.
	//GL_DYNAMIC_DRAW : the data is changed a lot and used many times.

	//VAO
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//VBO
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//EBO
	unsigned int EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	//Set vertex attributes pointers

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
}

unsigned int loadImage(const char* path)
{
	SDL_Surface* surface = IMG_Load(path);
	if (!surface)
	{
		std::cerr << "Failed to load image: " << std::endl;
		return 0;
	}

	SDL_FlipSurface(surface, SDL_FLIP_VERTICAL);

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);

	SDL_DestroySurface(surface);

	ConstantTexture constantTexture;
	constantTexture.value = texture;
	GlobalTextureAdd(path, &constantTexture);

	return texture;
}

void bindTexture(unsigned int texture)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
}

void bindTexture(const char* key)
{
	ConstantTexture* texture = GlobalTextureGet(key);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture->value);
}

static void DrawSprite(Shader shader, unsigned int texture)
{
	bindTexture(texture);

	//Use shader program when rendering
	shader.use();

	shader.setInt("texture", 1);

	mat4x4 trans = mat4x4(1.0f);
	//trans = translate(trans, float3(1.0f, 0.0f, 0.0f));
	//trans = scale(trans, float3(0.5, 0.5, 0.5));
	//trans = rotationZ(trans, 0.5); 
	//print(trans);

	unsigned int transformLoc = glGetUniformLocation(shader.ID, "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, trans.elements);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	//glDrawArrays(GL_TRIANGLES, 0, 3);
	
	//unbinds?
	//vao unbnd before ebo
	//glDeleteVertexArrays(1, &VAO);
	//glDeleteBuffers(1, &VBO);
	//glDeleteBuffers(1, &EBO);
	//glDeleteProgram(shaderProgram);
}

struct xxx_renderer* xxx_rendererCreate(SDL_Window* window)
{
	xxx_renderer* renderer = (xxx_renderer*)SDL_malloc(sizeof(*renderer));

	// Set OpenGL attributes
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	// Create an OpenGL context
	renderer->glContext = SDL_GL_CreateContext(window);

	if (renderer->glContext == nullptr)
	{
		std::cerr << "SDL_GL_CreateContext Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return nullptr;
	}

	// Initialise GLAD
	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD" << std::endl;
		SDL_Quit();
		return nullptr;
	}
	int windowWidth, windowHeight;
	SDL_GetWindowSize(window, &windowWidth, &windowHeight);

	glViewport(0, 0, windowWidth, windowHeight);

	createPipeline(&renderer->rectPipeline);
	createPipeline(&renderer->circlePipeline);
	createPipeline(&renderer->spritePipeline);

	return renderer;
}
//these move to cpp?

void createPipeline(xxx_graphicsPipeline* pipeline)//pass paths here
{
	bool wirefame = false;

	bool vsExists = std::filesystem::exists(GL_RESOURCE_DIRECTORY_PATH"/shaders/learning/triangle.vs");
	SDL_assert(vsExists && "Shader file does not exist");
	bool fExists = std::filesystem::exists(GL_RESOURCE_DIRECTORY_PATH"/shaders/learning/triangle.fs");
	SDL_assert(fExists && "Shader file does not exist");
	
	//todo movew this shit out of the class
	Shader triangleShader(GL_RESOURCE_DIRECTORY_PATH"/shaders/learning/triangle.vs", GL_RESOURCE_DIRECTORY_PATH"/shaders/learning/triangle.fs");

	GetSimple2dShader();

	unsigned int texutre = loadImage(GL_RESOURCE_DIRECTORY_PATH"/assets/textures/demon.png");

	pipeline->program.id = triangleShader.ID;
}

void xxx_rendererDestroy(xxx_renderer* renderer)
{
	SDL_GL_DestroyContext(renderer->glContext);
	SDL_free(renderer);
};

xxx_api* xxx_load()
{
	return nullptr;
}

int main()
{
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) == false)
	{
		std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return -1;
	}

	int windowWidth = 800;
	int windowHeight = 600;

	// Create an SDL window
	SDL_Window* window = SDL_CreateWindow("Close it.", windowWidth, windowHeight, SDL_WINDOW_OPENGL);
	if (window == nullptr)
	{
		std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return -1;
	}
	
	xxx_renderer* renderer = xxx_rendererCreate(window);

	// Main loop flag
	bool quit = false;

	// Event handler
	SDL_Event e;

	
	while (!quit)
	{
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_EVENT_QUIT)
			{
				quit = true;
			}

			if (e.type == SDL_EVENT_WINDOW_RESIZED)
			{
				glViewport(0, 0, e.window.data1, e.window.data2);
			}

			if (e.type == SDL_EVENT_KEY_DOWN)
			{
				if (e.key.key == SDLK_ESCAPE)
				{
					quit = true;
				}
			}
		}

		if (wirefame)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}

		// Clear screen
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//this goes somewheer?
		DrawSprite(triangleShader, texutre);

		// Update window
		SDL_GL_SwapWindow(window);
	}

	// Clean up
	xxx_rendererDestroy(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

