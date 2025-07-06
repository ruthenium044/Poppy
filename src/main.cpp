#include <SDL3/SDL.h>
#include <glad/glad.h>
#include <iostream>
#include <filesystem>
#include "shader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static void TriangleShader()
{
	float verticesIndices[] = 
	{
		 0.5f,  0.5f, 0.0f,  // top right
		 0.5f, -0.5f, 0.0f,  // bottom right
		-0.5f, -0.5f, 0.0f,  // bottom left
		-0.5f,  0.5f, 0.0f   // top left 
	};

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
	//unsigned int VAO;
	//glGenVertexArrays(1, &VAO);
	//glBindVertexArray(VAO);
	// 
	//VBO
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


	//EBO
	unsigned int EBO;
	//glGenBuffers(1, &EBO);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	//Set vertex attributes pointers

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

}

unsigned int loadAndBindImage()
{
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// load and generate the texture
	int width, height, nrChannels;
	unsigned char* data = stbi_load(GL_RESOURCE_DIRECTORY_PATH"/assets/textures/demon.png", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	return texture;
}

static void DrawTriangle(Shader shader, unsigned int VAO)
{
	auto texture = loadAndBindImage();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	//Use shader program when rendering
	shader.use();

	shader.setInt("texture", 1);

	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(VAO);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	//glDrawArrays(GL_TRIANGLES, 0, 3);

	//unbinds?
	//vao unbnd before ebo
	//glDeleteVertexArrays(1, &VAO);
	//glDeleteBuffers(1, &VBO);
	//glDeleteBuffers(1, &EBO);
	//glDeleteProgram(shaderProgram);
}

int main()
{
	//LISTO TESTs
	/*HeapAllocato heapAllocato;
	{
		Listo listo(&heapAllocato, 5);
		listo.pushBack(1);
		SDL_assert(listo[0] == 1);
		SDL_assert(listo.getSize() == 1);
		listo.popBack();
		SDL_assert(listo.getSize() == 0);

		listo.pushBack(2);
		listo.pushBack(3);
		SDL_assert(listo.getSize() == 2);
		listo.insert(0, 4);
		SDL_assert(listo[0] == 4);
		listo.remove(0);
		SDL_assert(listo[0] == 2);
		listo.clear();
		SDL_assert(listo.isEmpty());

		for (auto element : listo)
		{
			SDL_assert(false && "Listo should be empty here");
		}
	}
	{
		Listo listo(&heapAllocato, 5);
		Listo listo2 = listo;

		listo2.pushBack(1);
		SDL_assert(listo2[0] == 1);
		SDL_assert(listo2.getSize() == 1);
		listo2.popBack();
		SDL_assert(listo2.getSize() == 0);

		listo2.pushBack(2);
		listo2.pushBack(3);
		SDL_assert(listo2.getSize() == 2);
		listo2.insert(0, 4);
		SDL_assert(listo2[0] == 4);
		listo2.remove(0);
		SDL_assert(listo2[0] == 2);
		listo2.clear();
		SDL_assert(listo2.isEmpty());
	}
	*/

	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) == false) 
	{
		std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return -1;
	}

	// Set OpenGL attributes
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

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

	// Create an OpenGL context
	SDL_GLContext glContext = SDL_GL_CreateContext(window);
	if (glContext == nullptr)
	{
		std::cerr << "SDL_GL_CreateContext Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return -1;
	}

	// Initialize GLAD
	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD" << std::endl;
		SDL_Quit();
		return -1;
	}

	glViewport(0, 0, windowWidth, windowHeight);

	// Main loop flag
	bool quit = false;

	// Event handler
	SDL_Event e;

	bool wirefame = false;

	//todo move out
	//create triangle
	bool vsExists = std::filesystem::exists(GL_RESOURCE_DIRECTORY_PATH"/shaders/learning/triangle.vs");
	SDL_assert(vsExists && "Shader file does not exist");
	bool fExists = std::filesystem::exists(GL_RESOURCE_DIRECTORY_PATH"/shaders/learning/triangle.fs");
	SDL_assert(fExists && "Shader file does not exist");

	Shader triangleShader(GL_RESOURCE_DIRECTORY_PATH"/shaders/learning/triangle.vs", GL_RESOURCE_DIRECTORY_PATH"/shaders/learning/triangle.fs");
	//todo move this lol
	//VAO
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	TriangleShader();
	
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

		if(wirefame)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}

		// Clear screen
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		DrawTriangle(triangleShader, VAO);

		// Update window
		SDL_GL_SwapWindow(window);
	}

	// Clean up
	SDL_GL_DestroyContext(glContext);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
