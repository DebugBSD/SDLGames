/*This source code copyrighted by Lazy Foo' Productions (2004-2020)
and may not be redistributed without written permission.*/

//Using SDL and standard IO
#include <windows.h>
#include <SDL.h>
#include <stdio.h>

/*
 * Constants 
 */

//Screen dimension constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

/*
 * Definitions
 */

struct GameApplication
{
	SDL_Window* m_pWindow;
	SDL_Renderer* m_pRenderer;
};

/* 
 * Globals
 */

GameApplication gGame;

/*
 * Functions 
 */

bool initSDL()
{
	bool success = true;
	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Create window
		gGame.m_pWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gGame.m_pWindow == NULL)
		{
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
			//Get window surface
			gGame.m_pRenderer = SDL_CreateRenderer(gGame.m_pWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (gGame.m_pRenderer == NULL)
			{
				printf("Unable to create a renderer! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
		}
	}

	return success;
}

void handleInput()
{
	SDL_Event e;

	while (SDL_PollEvent(&e) != 0)
	{
		switch (e.type)
		{
		case SDL_QUIT:
			exit(0);
			break;
		default:
			break;
		}
	}
}

void updateGame()
{
	SDL_SetRenderDrawColor(gGame.m_pRenderer, 96, 128,255, 255);
	SDL_RenderClear(gGame.m_pRenderer);
}

void draw()
{
	SDL_RenderPresent(gGame.m_pRenderer);
}

int main( int argc, char* args[] )
{
	int exitCode = EXIT_SUCCESS;
	if (!initSDL())
	{
		exitCode = EXIT_FAILURE;
	}
	else
	{
		// Gameloop
		while (true)
		{
			handleInput();
			updateGame();
			draw();
		}

		//Quit SDL subsystems
		SDL_Quit();

	}
	
	return exitCode;
}