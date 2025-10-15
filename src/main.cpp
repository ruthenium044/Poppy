#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <glad/glad.h>
#include <iostream>
#include <filesystem>
#include "SDL3/SDL_stdinc.h"
#include "shader.h"

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

