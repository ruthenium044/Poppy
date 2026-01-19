#include "poppy.h"

#include "SDL3/SDL_stdinc.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include "float3.h"
#include "mat4x4.h"

#include <assert.h>
#include <filesystem>
#include <fstream>
#include <glad/glad.h>
#include <iostream>
#include <sstream>

#define vertexDescSize 8
#define uniformDescSize 16

// Todo, make whole binding entry a macro for niceness
#define nameof(type, member) sizeof(((type *)NULL)->member) ? #member : ""

struct ppy_program
{
    unsigned int id;
};

struct LayoutDesc
{
    int size;
    int type; // I.e., GL_FLOAT or GL_INT
    bool normalized;
    int stride;
    const void *pointer;
};

struct VertexElement
{
    LayoutDesc desc[vertexDescSize];
    size_t count;
};

enum class DataType
{
    BOOL,
    INT,
    FLOAT,
    FLOAT3,
    MAT4x4,
};

struct UniformBinding
{
    DataType type;
    const char *name;
    size_t offset;
};

struct UniformDesc
{
    UniformBinding desc[uniformDescSize];
    size_t count;
};

struct ppy_graphicsPipeline
{
    ppy_program program;
    VertexElement vertexElement;
    unsigned int VBO;
    unsigned int VAO;

    // how uniform binding looks like
    UniformDesc uniformDescs[64];
    size_t uniformDescCount;
};

struct ppy_lightPipeline
{
    ppy_graphicsPipeline gpuPipeline;

    //uniform binding data
    struct LightUniform
    {
        float3 color;
        mat4x4 transform;
    };
    LightUniform uniforms[64];
    size_t uniformCount;
};

struct ppy_rectPipeline
{
    ppy_graphicsPipeline gpuPipeline;

   struct RectUniform
    {
        float3 objectColor;
        float3 lightColor;
        float3 lightPos;
        int32_t texture;
        mat4x4 model;
        mat4x4 view;
        mat4x4 projection;
    };
    RectUniform uniforms[64];
    size_t count;
};

struct ppy_graphicsSpritePipeline
{
    ppy_graphicsPipeline gpuPipeline;

    struct SpritePipelineUniform
    {
        float3 colour;
        mat4x4 transform;
    };
    SpritePipelineUniform uniforms[64];
    size_t count;
};

struct ppy_renderer
{
    SDL_GLContext glContext;
    ppy_rectPipeline rectPipeline;
    ppy_lightPipeline lightPipeline;
    ppy_graphicsPipeline circlePipeline;
    ppy_graphicsSpritePipeline spritePipeline;

    unsigned int texture;

    mat4x4 projection; 
};

// ==========================================================
// Shader
// ==========================================================

void bindTexture(unsigned int texture)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
}

unsigned int loadImage(const char *path)
{
    SDL_Surface *surface = IMG_Load(path);
    if(!surface)
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
    if(type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                      << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if(!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                      << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}

static void compileShader(unsigned int shader, const char *shaderSource)
{
    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);
    checkCompileErrors(shader, "SHADER");

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if(!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
}

unsigned int createProgram(unsigned int shaders[], int count)
{
    unsigned int shaderProgram = glCreateProgram();

    for(int i = 0; i < count; i++)
    {
        glAttachShader(shaderProgram, shaders[i]);
    }
    glLinkProgram(shaderProgram);
    checkCompileErrors(shaderProgram, "PROGRAM");

    int success;
    char infoLog[512];
    glGetShaderiv(shaderProgram, GL_COMPILE_STATUS, &success);

    if(!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    for(int i = 0; i < count; i++)
    {
        glDeleteShader(shaders[i]);
    }

    return shaderProgram;
}

static void CompileAndLinkShaders(unsigned int &shaderProgram, const char *vertexShaderSource,
                                  const char *fragmentShaderSource)
{
    // Create and compile shaders
    int legth = 0;
    unsigned int shaders[2] = {0, 0};
    if(vertexShaderSource)
    {
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER); // create shadre of type
        compileShader(vertexShader, vertexShaderSource);
        shaders[0] = vertexShader;
        legth++;
    }

    if(fragmentShaderSource)
    {
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        compileShader(fragmentShader, fragmentShaderSource);
        shaders[1] = fragmentShader;
        legth++;
    }

    // Create and link shader programs
    if(legth > 0)
    {
        shaderProgram = createProgram(shaders, SDL_arraysize(shaders));
    }
}

unsigned int createShader(const char *vertexPath, const char *fragmentPath)
{
    std::ifstream vertexFile;
    std::ifstream shaderFile;

    const char *vertexCode = nullptr;
    std::string vertexStream;

    const char *fragmentCode = nullptr;
    std::string fragmentStream;

    // todo func these
    if(vertexPath)
    {
        vertexFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
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
        shaderFile.open(fragmentPath);

        std::stringstream fShaderStream;
        fShaderStream << shaderFile.rdbuf();

        shaderFile.close();

        fragmentStream = fShaderStream.str();
        fragmentCode = fragmentStream.c_str();
    }

    // todo pass in array??
    unsigned int ID;
    CompileAndLinkShaders(ID, vertexCode, fragmentCode);
    return ID;
}

// ==========================================================
// VBO VAO
// ==========================================================

static unsigned int GenerateCubeVBO() // todo this needs to be made once per permutation and given out by need or
                                      // smthng?
{
    // GL_STREAM_DRAW: the data is set only once and used by the GPU at most a few times.
    // GL_STATIC_DRAW : the data is set only once and used many times.
    // GL_DYNAMIC_DRAW : the data is changed a lot and used many times.

    float vertices[] = {-0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.5f,  -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f,
                        0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, 0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f,
                        -0.5f, 0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f,

                        -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.5f,  -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,
                        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
                        -0.5f, 0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,

                        -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  -0.5f, 0.5f,  -0.5f, -1.0f, 0.0f,  0.0f,
                        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,  -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,
                        -0.5f, -0.5f, 0.5f,  -1.0f, 0.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,

                        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.5f,  0.5f,  -0.5f, 1.0f,  0.0f,  0.0f,
                        0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,  0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,
                        0.5f,  -0.5f, 0.5f,  1.0f,  0.0f,  0.0f,  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

                        -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.5f,  -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,
                        0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,
                        -0.5f, -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,

                        -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,
                        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
                        -0.5f, 0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f};

    // VBO
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    return VBO;
}

static unsigned int CreateVAO(const LayoutDesc *desc, size_t count)
{
    // Stores vertex attribute config
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    for(int index = 0; index < count; index++)
    {
        glVertexAttribPointer(index, desc->size, desc->type, desc->normalized, desc->stride, desc->pointer);
        glEnableVertexAttribArray(index);
    }
    return VAO;
}

// ==========================================================
// Uniforms
// ==========================================================

void setBool(unsigned int programId, const std::string &name, void *value)
{
    glUniform1i(glGetUniformLocation(programId, name.c_str()), *(int *)value);
}

void setInt(unsigned int programId, const std::string &name, void *value)
{
    glUniform1i(glGetUniformLocation(programId, name.c_str()), *(int *)value);
}

void setFloat(unsigned int programId, const std::string &name, void *value)
{
    glUniform1f(glGetUniformLocation(programId, name.c_str()), *(float *)value);
}

void setFloat3(unsigned int programId, const std::string &name, void *value)
{
    float3 valueFloat3 = *(float3 *)value;
    glUniform3f(glGetUniformLocation(programId, name.c_str()), valueFloat3.x, valueFloat3.y, valueFloat3.z);
}

void setMat4x4(unsigned int programId, const std::string &name, void *value)
{
    mat4x4 valueMat4x4 = *(mat4x4 *)value;
    glUniformMatrix4fv(glGetUniformLocation(programId, name.c_str()), 1, GL_FALSE, valueMat4x4.elements);
}

static void setUniform(unsigned int programId, const UniformDesc *uniformDescriptions, void *data)
{
    char *bytes = (char *)data;

    for(size_t bindingIndex = 0; bindingIndex < uniformDescriptions->count; bindingIndex++)
    {
        const UniformBinding binding = uniformDescriptions->desc[bindingIndex];
        char *dataAt = bytes + (size_t)binding.offset;
        switch(binding.type)
        {
        case DataType::BOOL:
            setBool(programId, binding.name, dataAt);
            break;
        case DataType::INT:
            setInt(programId, binding.name, dataAt);
            break;
        case DataType::FLOAT:
            setFloat(programId, binding.name, dataAt);
            break;
        case DataType::FLOAT3:
            setFloat3(programId, binding.name, dataAt);
            break;
        case DataType::MAT4x4:
            setMat4x4(programId, binding.name, dataAt);
            break;
        default:
            SDL_assert(false && "Binding type doesnt exist");
            break;
        }
    }
}

void createPipeline(ppy_graphicsPipeline *pipeline, const char *vertexPath = nullptr,
                    const char *fragmentPath = nullptr)
{
    if(vertexPath)
    {
        bool vsExists = std::filesystem::exists(vertexPath);
        SDL_assert(vsExists && "Shader file does not exist");
    }

    if(fragmentPath)
    {
        bool fExists = std::filesystem::exists(fragmentPath);
        SDL_assert(fExists && "Shader file does not exist");
    }

    pipeline->program.id = createShader(vertexPath, fragmentPath);

    pipeline->VBO = GenerateCubeVBO();
    pipeline->VAO = CreateVAO(pipeline->vertexElement.desc, pipeline->vertexElement.count);
}

// ==========================================================
// API
// ==========================================================

ppy_renderer *rendererCreate(SDL_Window *window)
{
    ppy_renderer *renderer = (ppy_renderer *)SDL_malloc(sizeof(*renderer));
    memset(renderer, 0, sizeof(*renderer));

    // Set OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Create an OpenGL context
    renderer->glContext = SDL_GL_CreateContext(window);

    if(renderer->glContext == nullptr)
    {
        std::cerr << "SDL_GL_CreateContext Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return nullptr;
    }

    // Initialise GLAD
    if(!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        SDL_Quit();
        return nullptr;
    }
    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    glViewport(0, 0, windowWidth, windowHeight);
    int dim[4];
    glGetIntegerv(GL_VIEWPORT, dim);
    assert(dim[2] == windowWidth && dim[3] == windowHeight);

    glEnable(GL_DEPTH_TEST);

    constexpr int vertexSize = 6;

    VertexElement rectDesc = {
        .desc =
            {
                {3, GL_FLOAT, GL_FALSE, vertexSize * sizeof(float), (void *)0},
                {3, GL_FLOAT, GL_FALSE, vertexSize * sizeof(float), (void *)(3 * sizeof(float))},
            },
        .count = 2,
    };
    renderer->rectPipeline.gpuPipeline.vertexElement = rectDesc;
    createPipeline(&renderer->rectPipeline.gpuPipeline, GL_RESOURCE_DIRECTORY_PATH "/shaders/learning/triangle.vs",
                   GL_RESOURCE_DIRECTORY_PATH "/shaders/learning/triangle.fs");

    VertexElement lightDesc = {
        .desc =
            {
                {3, GL_FLOAT, GL_FALSE, vertexSize * sizeof(float), (void *)0},
                {3, GL_FLOAT, GL_FALSE, vertexSize * sizeof(float), (void *)(3 * sizeof(float))},
            },
        .count = 2,
    };
    renderer->lightPipeline.gpuPipeline.vertexElement = lightDesc;
    createPipeline(&renderer->lightPipeline.gpuPipeline, GL_RESOURCE_DIRECTORY_PATH "/shaders/learning/light.vs",
                   GL_RESOURCE_DIRECTORY_PATH "/shaders/learning/light.fs");

    createPipeline(&renderer->circlePipeline);
    createPipeline(&renderer->spritePipeline.gpuPipeline);

    renderer->texture = loadImage(GL_RESOURCE_DIRECTORY_PATH "/assets/textures/demon.png");

    return renderer;
}

static void createUniforms(ppy_renderer *renderer)
{
    //todo function draw rect, light and whatever else for each pipeline
    //then make draw model with certain shader

    UniformDesc lightBindings{
        .desc =
            {
                {DataType::FLOAT3, nameof(ppy_lightPipeline::LightUniform, color),
                 offsetof(ppy_lightPipeline::LightUniform, color)},
                {DataType::MAT4x4, nameof(ppy_lightPipeline::LightUniform, transform),
                 offsetof(ppy_lightPipeline::LightUniform, transform)},
            },
        .count = 2,
    };
    renderer->lightPipeline.gpuPipeline.uniformDescs[0] = lightBindings;
    renderer->lightPipeline.gpuPipeline.uniformDescCount = 1;

    UniformDesc rectBindings{
        .desc =
            {
                {DataType::FLOAT3, nameof(ppy_rectPipeline::RectUniform, objectColor),
                 offsetof(ppy_rectPipeline::RectUniform, objectColor)},
                {DataType::FLOAT3, nameof(ppy_rectPipeline::RectUniform, lightColor),
                 offsetof(ppy_rectPipeline::RectUniform, lightColor)},
                {DataType::FLOAT3, nameof(ppy_rectPipeline::RectUniform, lightPos),
                 offsetof(ppy_rectPipeline::RectUniform, lightPos)},
                {DataType::INT, nameof(ppy_rectPipeline::RectUniform, texture),
                 offsetof(ppy_rectPipeline::RectUniform, texture)}, // todo texture binding?
                {DataType::MAT4x4, nameof(ppy_rectPipeline::RectUniform, model),
                 offsetof(ppy_rectPipeline::RectUniform, model)},
                {DataType::MAT4x4, nameof(ppy_rectPipeline::RectUniform, view),
                 offsetof(ppy_rectPipeline::RectUniform, view)},
                {DataType::MAT4x4, nameof(ppy_rectPipeline::RectUniform, projection),
                 offsetof(ppy_rectPipeline::RectUniform, projection)},
            },
        .count = 7,
    };
    renderer->rectPipeline.gpuPipeline.uniformDescs[0] = rectBindings;
    renderer->rectPipeline.gpuPipeline.uniformDescCount = 1; // todo when to set this count?

    UniformDesc greenRectBindings{
        .desc =
            {
                {DataType::FLOAT3, nameof(ppy_rectPipeline::RectUniform, objectColor),
                 offsetof(ppy_rectPipeline::RectUniform, objectColor)},
                {DataType::FLOAT3, nameof(ppy_rectPipeline::RectUniform, lightColor),
                 offsetof(ppy_rectPipeline::RectUniform, lightColor)},
                {DataType::FLOAT3, nameof(ppy_rectPipeline::RectUniform, lightPos),
                 offsetof(ppy_rectPipeline::RectUniform, lightPos)},
                {DataType::INT, nameof(ppy_rectPipeline::RectUniform, texture),
                 offsetof(ppy_rectPipeline::RectUniform, texture)}, // todo texture binding?
                {DataType::MAT4x4, nameof(ppy_rectPipeline::RectUniform, model),
                 offsetof(ppy_rectPipeline::RectUniform, model)},
                {DataType::MAT4x4, nameof(ppy_rectPipeline::RectUniform, view),
                 offsetof(ppy_rectPipeline::RectUniform, view)},
                {DataType::MAT4x4, nameof(ppy_rectPipeline::RectUniform, projection),
                 offsetof(ppy_rectPipeline::RectUniform, projection)},
            },
        .count = 7,
    };
    renderer->rectPipeline.gpuPipeline.uniformDescs[1] = greenRectBindings;
    renderer->rectPipeline.gpuPipeline.uniformDescCount = 2;
}

static void drawPipelines(ppy_renderer *renderer)
{
    //todo this once or always?
    createUniforms(renderer);

    float3 lightPos = float3(0.6f, 0.5f, -1.0f);
    float3 lightColor = float3(1.0f, 1.0f, 1.0f);

    mat4x4 newTrLight = mat4x4(1.0f);
    newTrLight = translate(newTrLight, lightPos);
    newTrLight = scale(newTrLight, float3(0.25, 0.25, 0.25));

    mat4x4 trLight = newTrLight;

    // CUBE
    float3 objectColor = float3(1.0f, 0.5f, 0.31f);
    int texture = 1;

    mat4x4 model = mat4x4(1.0f);
    model = scale(model, float3(0.5, 0.5, 0.5));
    model = rotationX(model, (float)SDL_GetTicks() * 0.0002);
    model = rotationY(model, (float)SDL_GetTicks() * 0.0002);

    mat4x4 model2 = mat4x4(1.0f);
    model2 = translate(model2, float3(-1, 0, 0));
    model2 = scale(model2, float3(0.5, 0.5, 0.5));
    model2 = rotationX(model2, (float)SDL_GetTicks() * 0.0002);
    model2 = rotationY(model2, (float)SDL_GetTicks() * 0.0002);

    mat4x4 view = mat4x4(1.0f);
    view = translate(view, float3(0.0f, 0.0f, -3.0f));

    ppy_lightPipeline::LightUniform lightElement = {
        .color = lightColor,
        .transform = trLight,
    };
    ppy_rectPipeline::RectUniform rectElement = {
        .objectColor = objectColor,
        .lightColor = lightColor,
        .lightPos = lightPos,
        .texture = texture,
        .model = model,
        .view = view,
        .projection = projection,
    };
    ppy_rectPipeline::RectUniform greenRectElement = {
        .objectColor = float3(0,1,0),
        .lightColor = lightColor,
        .lightPos = lightPos,
        .texture = texture,
        .model = model2,
        .view = view,
        .projection = projection,
    };

    //light
    glUseProgram(renderer->lightPipeline.gpuPipeline.program.id);
    glBindVertexArray(renderer->lightPipeline.gpuPipeline.VAO);
    setUniform(renderer->lightPipeline.gpuPipeline.program.id, &renderer->lightPipeline.gpuPipeline.uniformDescs[0], &lightElement);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // todo check if texture ever works again lol
    bindTexture(renderer->texture);
    glBindTexture(GL_TEXTURE_2D, renderer->texture);

    //rect
    glUseProgram(renderer->rectPipeline.gpuPipeline.program.id);
    glBindVertexArray(renderer->rectPipeline.gpuPipeline.VAO);

    // todo is this needed here?
    //glBindBuffer(GL_ARRAY_BUFFER, renderer->rectPipeline.gpuPipeline.VBO);

    setUniform(renderer->rectPipeline.gpuPipeline.program.id, &renderer->rectPipeline.gpuPipeline.uniformDescs[0], &rectElement);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    //green rect
    glUseProgram(renderer->rectPipeline.gpuPipeline.program.id);
    glBindVertexArray(renderer->rectPipeline.gpuPipeline.VAO);

    setUniform(renderer->rectPipeline.gpuPipeline.program.id, &renderer->rectPipeline.gpuPipeline.uniformDescs[1],
               &greenRectElement);
    glDrawArrays(GL_TRIANGLES, 0, 36);


    glBindVertexArray(0);

    // unbinds?
    // vao unbnd before ebo
    // glDeleteVertexArrays(1, &VAO);
    // glDeleteBuffers(1, &VBO);
    // glDeleteBuffers(1, &EBO);
    // glDeleteProgram(shaderProgram);
}

static void rendererDestroy(ppy_renderer *renderer)
{
    SDL_GL_DestroyContext(renderer->glContext);
    SDL_free(renderer);
};

static ppy_api api = {.create = rendererCreate, .draw = drawPipelines, .destroy = rendererDestroy};

ppy_api *ppy_get()
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
