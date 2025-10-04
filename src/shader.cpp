#include "shader.h"

#include "SDL3/SDL_stdinc.h"

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

static void CompileAndLinkShaders(unsigned int& shaderProgram, const char* vertexShaderSource, const char* fragmentShaderSource)
{
	//Create and compile shaders
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER); //create shadre of type
	compileShader(vertexShader, vertexShaderSource);

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	compileShader(fragmentShader, fragmentShaderSource);

	//Create and link shader programs
	unsigned int shaders[] = { vertexShader, fragmentShader }; //these can be specified

	shaderProgram = createProgram(shaders, SDL_arraysize(shaders));
}

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    std::string vertexStream;
    std::string fragmentStream;
    std::ifstream vertexFile;
    std::ifstream shaderFile;
  
    vertexFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try //todo how to make this not try catch?
    {
        vertexFile.open(vertexPath);
        shaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
       
        vShaderStream << vertexFile.rdbuf();
        fShaderStream << shaderFile.rdbuf();
      
        vertexFile.close();
        shaderFile.close();
     
		vertexStream = vShaderStream.str();
		fragmentStream = fShaderStream.str();
    }
    catch (std::ifstream::failure e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    const char* vertexCode = vertexStream.c_str();
    const char* fragmentCode = fragmentStream.c_str();

	CompileAndLinkShaders(ID, vertexCode, fragmentCode);
}

void Shader::use()
{
	glUseProgram(ID);
}

void Shader::setBool( const std::string& name, bool value ) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt( const std::string& name, int value ) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat( const std::string& name, float value ) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

//todo set float3