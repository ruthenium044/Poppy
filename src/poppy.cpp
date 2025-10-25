#include "shader.h"

#include "SDL3/SDL_stdinc.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <glad/glad.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include "SDL3/SDL_stdinc.h"
#include "float3.h"
#include "float4.h"
#include "mat4x4.h"
#include <stddef.h>

struct ppy_buffer
{

};

struct ppy_shader
{

};

struct ppy_program
{
	unsigned int id;
};

struct ppy_graphicsPipeline
{
	ppy_program program;
	
};

struct ppy_renderer
{
	SDL_GLContext glContext;
	ppy_graphicsPipeline rectPipeline;
	ppy_graphicsPipeline circlePipeline;
	ppy_graphicsPipeline spritePipeline;
};

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

void checkCompileErrors(unsigned int shader, std::string type)
{
	int success;
	char infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
}

static void compileShader(unsigned int shader, const char* shaderSource)
{
	glShaderSource(shader, 1, &shaderSource, NULL);
	glCompileShader(shader);
	checkCompileErrors(shader, "SHADER");

	int  success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
}

unsigned int createProgram(unsigned int shaders[], int count)
{
	unsigned int shaderProgram = glCreateProgram();

	for (int i = 0; i < count; i++)
	{
		glAttachShader(shaderProgram, shaders[i]);
	}
	glLinkProgram(shaderProgram);
	checkCompileErrors(shaderProgram, "PROGRAM");

	int  success;
	char infoLog[512];
	glGetShaderiv(shaderProgram, GL_COMPILE_STATUS, &success);

	if (!success) 
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	for (int i = 0; i < count; i++)
	{
		glDeleteShader(shaders[i]);
	}

	return shaderProgram;
}

static void CompileAndLinkShaders(unsigned int& shaderProgram, const char* vertexShaderSource, const char* fragmentShaderSource)
{
	//Create and compile shaders
	unsigned int shaders[2];
	if (vertexShaderSource)
	{
		unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER); //create shadre of type
		compileShader(vertexShader, vertexShaderSource);
		shaders[0] = vertexShader;
	}

	if (fragmentShaderSource)
	{
		unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		compileShader(fragmentShader, fragmentShaderSource);
		shaders[1] = fragmentShader;
	}

	//Create and link shader programs
	shaderProgram = createProgram(shaders, SDL_arraysize(shaders));
}

unsigned int createShader(const char* vertexPath, const char* fragmentPath)
{
    std::ifstream vertexFile;
    std::ifstream shaderFile;
  
	const char* vertexCode;
    const char* fragmentCode;

	//todo func these
	if(vertexPath)
	{
		vertexFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		std::string vertexStream;
		vertexFile.open(vertexPath);

		std::stringstream vShaderStream;
		vShaderStream << vertexFile.rdbuf();

		vertexFile.close();

		vertexStream = vShaderStream.str();
		vertexCode = vertexStream.c_str();
	}
        
	if(fragmentPath)
	{
		shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		std::string fragmentStream;
        shaderFile.open(fragmentPath);

        std::stringstream fShaderStream;  
        fShaderStream << shaderFile.rdbuf();
      
        shaderFile.close();
     
		fragmentStream = fShaderStream.str();
		fragmentCode = fragmentStream.c_str();
	}

	//todo pass in array??
	unsigned int ID;
	CompileAndLinkShaders(ID, vertexCode, fragmentCode); 
	return ID;
}

void createPipeline(ppy_graphicsPipeline* pipeline, const char* vertexPath = nullptr, const char* fragmentPath = nullptr)
{
	bool wirefame = false;

	if( vertexPath )
	{
		bool vsExists = std::filesystem::exists(vertexPath);
		SDL_assert(vsExists && "Shader file does not exist");
	}
	
	if( fragmentPath )
	{
		bool fExists = std::filesystem::exists(fragmentPath);
		SDL_assert(fExists && "Shader file does not exist");
	}

	pipeline->program.id = createShader(vertexPath, fragmentPath);

	GetSimple2dShader();

	unsigned int texutre = loadImage(GL_RESOURCE_DIRECTORY_PATH"/assets/textures/demon.png");
}

struct ppy_renderer* rendererCreate(SDL_Window* window)
{
	ppy_renderer* renderer = (ppy_renderer*)SDL_malloc(sizeof(*renderer));

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

	createPipeline(&renderer->rectPipeline, GL_RESOURCE_DIRECTORY_PATH"/shaders/learning/triangle.vs", GL_RESOURCE_DIRECTORY_PATH"/shaders/learning/triangle.fs");
	createPipeline(&renderer->circlePipeline);
	createPipeline(&renderer->spritePipeline);

	return renderer;
}

static void rendererDestroy(ppy_renderer* renderer)
{
	SDL_GL_DestroyContext(renderer->glContext);
	SDL_free(renderer);
};

void use(ppy_program* program)
{
	glUseProgram(program->id);
}

void setBool( ppy_program* program, const std::string& name, bool value )
{
	glUniform1i(glGetUniformLocation(program->id, name.c_str()), (int)value);
}

void setInt(ppy_program* program, const std::string& name, int value )
{
	glUniform1i(glGetUniformLocation(program->id, name.c_str()), value);
}

void setFloat(ppy_program* program, const std::string& name, float value )
{
	glUniform1f(glGetUniformLocation(program->id, name.c_str()), value);
}

static void drawSprite()
{
	bindTexture(texture);

	//Use shader program when rendering
	use(program);

	setInt(program, "texture", 1);

	mat4x4 trans = mat4x4(1.0f);
	//trans = translate(trans, float3(1.0f, 0.0f, 0.0f));
	//trans = scale(trans, float3(0.5, 0.5, 0.5));
	//trans = rotationZ(trans, 0.5); 
	//print(trans);

	unsigned int transformLoc = glGetUniformLocation(program->id, "transform");
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

/*
enum class MaterialEditorShaderType
{
	Vertex,
	Fragment
};

enum class MaterialEditorShaderArgumentType
{
	Bool,
	Int,
	Float,
	Texture
};

struct MaterialEditorShaderArgument
{
	MaterialEditorShaderArgumentType type;
	union
	{
		bool asBool;
		int asInt;
		float asFloat;
		void* asTexture;
	};
};

struct MaterialEditorParameters
{
	MaterialEditorShaderArgument* arguments;
	int count;
};

struct MaterialEditor
{
	const char* path;
	MaterialEditorParameters parameters;
};
*/

static ppy_api api = {
	.create = rendererCreate,
	.draw = drawSprite,
	.destroy = rendererDestroy
};

ppy_api* ppy_get()
{
	return &api;
}
