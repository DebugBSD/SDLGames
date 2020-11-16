/*This source code copyrighted by Lazy Foo' Productions (2004-2020)
and may not be redistributed without written permission.*/

//Using SDL and standard IO
#include <windows.h>
#include <SDL.h>
#include <SDL_image.h>
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

struct Entity
{
	int m_x;
	int m_y;
	SDL_Texture* m_pTexture;
};

void blit(SDL_Texture* pTexture, int x, int y);

/* 
 * Globals
 */

GameApplication gGame;
Entity gPlayer;

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

			if (!IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG))
			{
				printf("Unable to init Image subsystem! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
		}
	}

	return success;
}

void closeSDL()
{
	SDL_DestroyWindow(gGame.m_pWindow);

	SDL_DestroyRenderer(gGame.m_pRenderer);

	IMG_Quit();
	SDL_Quit();

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

	// Draw the scene
	blit(gPlayer.m_pTexture, gPlayer.m_x, gPlayer.m_y);
}

SDL_Texture* loadTexture(const char *pFileName)
{
	SDL_Texture* pTexture = NULL;

	pTexture = IMG_LoadTexture(gGame.m_pRenderer,pFileName);

	return pTexture;
}

void blit(SDL_Texture* pTexture, int x, int y)
{
	SDL_Rect dest{ x,y };

	// This function retrieves the dimensions from the image.
	SDL_QueryTexture(pTexture, NULL, NULL, &dest.w, &dest.h);

	// Draws the texture on the coordinates specified into dest structure.
	SDL_RenderCopy(gGame.m_pRenderer, pTexture, NULL, &dest);
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
		gPlayer.m_x = 100;
		gPlayer.m_y = 100;
		gPlayer.m_pTexture = loadTexture("player.png");

		// Gameloop
		while (true)
		{
			handleInput();
			updateGame();
			
			draw();
		}

		//Quit SDL subsystems
		closeSDL();

	}
	
	return exitCode;
}