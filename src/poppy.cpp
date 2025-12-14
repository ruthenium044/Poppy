#include "poppy.h"

#include "SDL3/SDL_stdinc.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <glad/glad.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include "float3.h"
#include "mat4x4.h"

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
	VertexLayoutDescElement desc[];
	DataTypeBinding lightBindings[];
};

struct ppy_graphicsSpritePipeline
{
	ppy_graphicsPipeline gpuPipeline;
	unsigned int texutres;
};

struct ppy_renderer
{
	SDL_GLContext glContext;
	ppy_graphicsPipeline rectPipeline;
	ppy_graphicsPipeline lightPipeline;
	ppy_graphicsPipeline circlePipeline;
	ppy_graphicsPipeline spritePipeline;

	unsigned int texutre;
};

// ==========================================================
// Shader
// ==========================================================

void bindTexture(unsigned int texture)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
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

	return texture;
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
	int legth = 0;
	unsigned int shaders[2] = { 0, 0 };
	if (vertexShaderSource)
	{
		unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER); //create shadre of type
		compileShader(vertexShader, vertexShaderSource);
		shaders[0] = vertexShader;
		legth++;
	}

	if (fragmentShaderSource)
	{
		unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		compileShader(fragmentShader, fragmentShaderSource);
		shaders[1] = fragmentShader;
		legth++;
	}

	//Create and link shader programs
	if (legth > 0)
	{
		shaderProgram = createProgram(shaders, SDL_arraysize(shaders));
	}
}

unsigned int createShader(const char* vertexPath, const char* fragmentPath)
{
	std::ifstream vertexFile;
	std::ifstream shaderFile;

	const char* vertexCode = nullptr;
	std::string vertexStream;

	const char* fragmentCode = nullptr;
	std::string fragmentStream;

	//todo func these
	if (vertexPath)
	{
		vertexFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		vertexFile.open(vertexPath);

		std::stringstream vShaderStream;
		vShaderStream << vertexFile.rdbuf();

		vertexFile.close();

		vertexStream = vShaderStream.str();
		vertexCode = vertexStream.c_str();
	}

	if (fragmentPath)
	{
		shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
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

void setBool(ppy_program* program, const std::string& name, bool value)
{
	glUniform1i(glGetUniformLocation(program->id, name.c_str()), (int)value);
}

void setInt(ppy_program* program, const std::string& name, int value)
{
	glUniform1i(glGetUniformLocation(program->id, name.c_str()), value);
}

void setFloat(ppy_program* program, const std::string& name, float value)
{
	glUniform1f(glGetUniformLocation(program->id, name.c_str()), value);
}

void setFloat3(ppy_program* program, const std::string& name, float3 value)
{
	glUniform3f(glGetUniformLocation(program->id, name.c_str()), value.x, value.y, value.z);
}

void setMat4x4(ppy_program* program, const std::string& name, mat4x4 value)
{
	glUniformMatrix4fv(glGetUniformLocation(program->id, name.c_str()), 1, GL_FALSE, value.elements);
}

struct VertexLayoutDescElement
{
	int type; // I.e., GL_FLOAT or GL_INT
	int size;
	int normalized; // ...
	int stride;
};

struct VertexLayout
{
	unsigned int vao;
	// ...
};

static unsigned int CreateVAO(VertexLayoutDescElement* desc, size_t count)
{
	// Stores vertex attribute config
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	for (int index = 0; index < count; index++)
	{

		//move in
	}

	auto vertexSize = 6;

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertexSize * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertexSize * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	return VAO;
}

static unsigned int GenerateCubeVBO()
{
	//GL_STREAM_DRAW: the data is set only once and used by the GPU at most a few times.
	//GL_STATIC_DRAW : the data is set only once and used many times.
	//GL_DYNAMIC_DRAW : the data is changed a lot and used many times.

	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
	};

	//VBO
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	return VBO;
}

void createPipeline(ppy_graphicsPipeline* pipeline, const char* vertexPath = nullptr, const char* fragmentPath = nullptr)
{
	if (vertexPath)
	{
		bool vsExists = std::filesystem::exists(vertexPath);
		SDL_assert(vsExists && "Shader file does not exist");
	}

	if (fragmentPath)
	{
		bool fExists = std::filesystem::exists(fragmentPath);
		SDL_assert(fExists && "Shader file does not exist");
	}

	pipeline->program.id = createShader(vertexPath, fragmentPath);

	GenerateCubeVBO();

	CreateVAO(desc, SDL_arraysize(desc));


}

// ==========================================================
// API
// ==========================================================

ppy_renderer* rendererCreate(SDL_Window* window)
{
	ppy_renderer* renderer = (ppy_renderer*)SDL_malloc(sizeof(*renderer));

	// Set OpenGL attributes
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
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

	glEnable(GL_DEPTH_TEST);

	renderer->rectPipeline.desc =
	{
		{ GL_FLOAT, 3, GL_FALSE, sizeof(float) * 6},
		{ GL_FLOAT, 3, GL_FALSE, sizeof(float) * 6},
	};
	createPipeline(&renderer->rectPipeline, GL_RESOURCE_DIRECTORY_PATH"/shaders/learning/triangle.vs", GL_RESOURCE_DIRECTORY_PATH"/shaders/learning/triangle.fs");

	renderer->lightPipeline.desc =
	{
		{ GL_FLOAT, 3, GL_FALSE, sizeof(float) * 6},
		{ GL_FLOAT, 3, GL_FALSE, sizeof(float) * 6},
	};
	createPipeline(&renderer->lightPipeline, GL_RESOURCE_DIRECTORY_PATH"/shaders/learning/light.vs", GL_RESOURCE_DIRECTORY_PATH"/shaders/learning/light.fs");

	createPipeline(&renderer->circlePipeline);
	createPipeline(&renderer->spritePipeline);

	renderer->texutre = loadImage(GL_RESOURCE_DIRECTORY_PATH"/assets/textures/demon.png");

	return renderer;
}

enum DataType
{
	DATA_TYPE_FLOAT3,
	DATA_TYPE_MAT4x4,
	DATA_TYPE_INT,
};

struct DataTypeBinding
{
	DataType type;
	const char* name;
	void* ptr;
};

static void whatver(DataTypeBinding* lightBindings, size_t count)
{
	for (size_t index = 0; index < count; index++)
	{
		DataTypeBinding binding = lightBindings[index];
		switch (binding.type)
		{
		case DATA_TYPE_FLOAT3: break;
			setFloat3
		case DATA_TYPE_MAT4x4: break;
		case DATA_TYPE_INT: break;
		}
	}

}

static void drawSprite(ppy_renderer* renderer)
{

	float3 lightPos = float3(0.6f, 0.5f, 1.0f);
	float3 lightColor = float3(1.0f, 1.0f, 1.0f);

	mat4x4 trLight = mat4x4(1.0f);
	trLight = translate(trLight, lightPos);
	trLight = scale(trLight, float3(0.25, 0.25, 0.25));

	DataTypeBinding lightBindings[] =
	{
		{ DATA_TYPE_FLOAT3, "color", &lightColor},
		{ DATA_TYPE_MAT4x4, "transform", &trLight},
	};

	//for all stuff
	whatver(renderer->lightPipeline.lightBindings); //uniforms whatever
	
	mat4x4 model = mat4x4(1.0f);
	model = scale(model, float3(0.5, 0.5, 0.5));
	model = rotationX(model, (float)SDL_GetTicks() * 0.0002);
	model = rotationY(model, (float)SDL_GetTicks() * 0.0002);

	mat4x4 view = mat4x4(1.0f);
	//view = translate(view, float3(0.0f, 0.0f, -3.0f));

	//todo
	mat4x4 projection = mat4x4(1.0f);
	//projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	//// note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.

	//bindTexture(renderer->texutre);

	//lightbulb
	glUseProgram(renderer->lightPipeline.program.id);


	setFloat3(&renderer->lightPipeline.program, "color", lightColor);
	setMat4x4(&renderer->lightPipeline.program, "transform", trLight);

	glDrawArrays(GL_TRIANGLES, 0, 36);

	//cube itself

	//Use shader program when rendering
	glUseProgram(renderer->rectPipeline.program.id);

	setFloat3(&renderer->rectPipeline.program, "objectColor", float3(1.0f, 0.5f, 0.31f));
	setFloat3(&renderer->rectPipeline.program, "lightColor", lightColor);
	setFloat3(&renderer->rectPipeline.program, "lightPos", lightPos);

	setInt(&renderer->rectPipeline.program, "texture", 1);

	setMat4x4(&renderer->rectPipeline.program, "model", model);
	setMat4x4(&renderer->rectPipeline.program, "view", view);

	setMat4x4(&renderer->rectPipeline.program, "projection", projection);

	glBindTexture(GL_TEXTURE_2D, renderer->texutre);

	glDrawArrays(GL_TRIANGLES, 0, 36);

	//unbinds?
	//vao unbnd before ebo
	//glDeleteVertexArrays(1, &VAO);
	//glDeleteBuffers(1, &VBO);
	//glDeleteBuffers(1, &EBO);
	//glDeleteProgram(shaderProgram);
}

static void rendererDestroy(ppy_renderer* renderer)
{
	SDL_GL_DestroyContext(renderer->glContext);
	SDL_free(renderer);
};

static ppy_api api = {
	.create = rendererCreate,
	.draw = drawSprite,
	.destroy = rendererDestroy
};

ppy_api* ppy_get()
{
	return &api;
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

