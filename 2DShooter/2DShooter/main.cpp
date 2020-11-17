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

const int MAX_KEYBOARD_KEYS = 350;

const int PLAYER_SPEED = 4;
const int PLAYER_BULLET_SPEED = 16;
/*
 * Definitions
 */

struct Delegate
{
	void (*logic)(void);
	void (*draw)(void);
};

struct GameApplication
{
	SDL_Window* m_pWindow;
	SDL_Renderer* m_pRenderer;
	Delegate m_delegate;
	int keyboard[MAX_KEYBOARD_KEYS];
};

struct Entity
{
	float m_x;
	float m_y;
	int m_w;
	int m_h;
	float m_dx;
	float m_dy;
	int m_health;
	int m_reload;
	SDL_Texture* m_pTexture;
	Entity* m_pNext;
};

struct Stage {
	Entity fighterHead, * pFighterTail;
	Entity bulletHead, * pBulletTail;
};

void blit(SDL_Texture* pTexture, int x, int y);

/* 
 * Globals
 */

GameApplication gGame;
Stage gStage;
SDL_Texture* gBulletTexture = NULL;
SDL_Texture* gEnemyTexture = NULL;
Entity* pPlayer = NULL;
int enemySpawnerTimer;
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

SDL_Texture* loadTexture(const char* pFileName)
{
	SDL_Texture* pTexture = NULL;

	pTexture = IMG_LoadTexture(gGame.m_pRenderer, pFileName);

	return pTexture;
}

void doKeyDown(SDL_KeyboardEvent* pEvent)
{
	if (!pEvent->repeat && pEvent->keysym.scancode < MAX_KEYBOARD_KEYS)
	{
		gGame.keyboard[pEvent->keysym.scancode] = 1;
	}
}

void doKeyUp(SDL_KeyboardEvent* pEvent)
{
	if (!pEvent->repeat && pEvent->keysym.scancode < MAX_KEYBOARD_KEYS)
	{
		gGame.keyboard[pEvent->keysym.scancode] = 0;
	}
}

static void fireBullet()
{
	Entity* pBullet;

	pBullet = (Entity*)malloc(sizeof(Entity));
	memset(pBullet, 0, sizeof(Entity));
	gStage.pBulletTail->m_pNext = pBullet;
	gStage.pBulletTail = pBullet;

	pBullet->m_x = pPlayer->m_x;
	pBullet->m_y = pPlayer->m_y;
	pBullet->m_dx = PLAYER_BULLET_SPEED;
	pBullet->m_pTexture = gBulletTexture;
	SDL_QueryTexture(pBullet->m_pTexture, NULL, NULL, &pBullet->m_w, &pBullet->m_h);

	pBullet->m_y += (pPlayer->m_h / 2) - (pBullet->m_h / 2);
	pPlayer->m_reload = 8;
}

static void doPlayer(void)
{
	pPlayer->m_dx = pPlayer->m_dy = 0;

	if (pPlayer->m_reload > 0)					pPlayer->m_reload--;
	if (gGame.keyboard[SDL_SCANCODE_UP])		pPlayer->m_dy = -PLAYER_SPEED;
	if (gGame.keyboard[SDL_SCANCODE_DOWN])		pPlayer->m_dy = PLAYER_SPEED;
	if (gGame.keyboard[SDL_SCANCODE_LEFT])		pPlayer->m_dx = -PLAYER_SPEED;
	if (gGame.keyboard[SDL_SCANCODE_RIGHT])		pPlayer->m_dx = PLAYER_SPEED;
	if (gGame.keyboard[SDL_SCANCODE_LCTRL] && pPlayer->m_reload == 0)	fireBullet();

	pPlayer->m_x += pPlayer->m_dx;
	pPlayer->m_y += pPlayer->m_dy;
}

static void doBullets(void)
{
	Entity* b, * prev;

	prev = &gStage.bulletHead;

	for (b = gStage.bulletHead.m_pNext; b != NULL; b = b->m_pNext)
	{
		b->m_x += b->m_dx;
		b->m_y += b->m_dy;

		if (b->m_x > SCREEN_WIDTH)
		{
			if (b == gStage.pBulletTail)
			{
				gStage.pBulletTail = prev;
			}

			prev->m_pNext = b->m_pNext;
			free(b);
			b = prev;
		}

		prev = b;
	}
}

static void doFighters()
{
	Entity *e, * prev = NULL;

	for (e = gStage.fighterHead.m_pNext; e != NULL; e = e->m_pNext)
	{
		e->m_x += e->m_dx;
		e->m_y += e->m_dy;

		if (e != pPlayer && e->m_x < -e->m_w)
		{
			if (e == gStage.pFighterTail)
			{
				gStage.pFighterTail = prev;
			}

			prev->m_pNext = e->m_pNext;
			free(e);
			e = prev;
		}

		prev = e;
	}
}

static void spawnEnemies()
{
	Entity* enemy;

	if (--enemySpawnerTimer <= 0)
	{
		enemy = (Entity*)malloc(sizeof(Entity));
		memset(enemy, 0, sizeof(Entity));
		gStage.pFighterTail->m_pNext = enemy;
		gStage.pFighterTail = enemy;

		enemy->m_x = SCREEN_WIDTH;
		enemy->m_y = rand() % SCREEN_HEIGHT;
		enemy->m_pTexture = gEnemyTexture;
		SDL_QueryTexture(enemy->m_pTexture, NULL, NULL, &enemy->m_w, &enemy->m_h);

		enemy->m_dx = -(2 + rand() % 4);

		enemySpawnerTimer = 30 + (rand() % 60);
	}
}

static void logic()
{
	doPlayer();

	doFighters();

	doBullets();

	spawnEnemies();
}

static void drawPlayer()
{
	blit(pPlayer->m_pTexture, pPlayer->m_x, pPlayer->m_y);
}

static void drawBullets()
{
	Entity* b;
	for (b = gStage.bulletHead.m_pNext; b != NULL; b = b->m_pNext)
	{
		blit(b->m_pTexture, b->m_x, b->m_y);
	}
}

static void drawFighters()
{
	Entity* e;
	for (e = gStage.fighterHead.m_pNext; e != NULL; e = e->m_pNext)
	{
		blit(e->m_pTexture, e->m_x, e->m_y);
	}
}

static void draw()
{
	drawPlayer();

	drawBullets();

	drawFighters();
}

void initPlayer()
{
	pPlayer = (Entity*)malloc(sizeof(Entity));
	memset(pPlayer, 0, sizeof(Entity));
	gStage.pFighterTail->m_pNext = pPlayer;
	gStage.pFighterTail = pPlayer;

	pPlayer->m_x = 100.0f;
	pPlayer->m_y = 100.0f;
	pPlayer->m_pTexture = loadTexture("player.png");
	SDL_QueryTexture(pPlayer->m_pTexture, NULL, NULL, &pPlayer->m_w, &pPlayer->m_h);
}

void initStage()
{
	gGame.m_delegate.logic = logic;
	gGame.m_delegate.draw = draw;

	memset(&gStage, 0, sizeof(Stage));
	gStage.pFighterTail = &gStage.fighterHead; 
	gStage.pBulletTail = &gStage.bulletHead;

	initPlayer();

	gBulletTexture = loadTexture("playerBullet.png");
	gEnemyTexture = loadTexture("enemy.png");

	enemySpawnerTimer = 0;
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
		case SDL_KEYDOWN:
			doKeyDown(&e.key);
			break;
		case SDL_KEYUP:
			doKeyUp(&e.key);
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

}

void blit(SDL_Texture* pTexture, int x, int y)
{
	SDL_Rect dest{ x,y };

	// This function retrieves the dimensions from the image.
	SDL_QueryTexture(pTexture, NULL, NULL, &dest.w, &dest.h);

	// Draws the texture on the coordinates specified into dest structure.
	SDL_RenderCopy(gGame.m_pRenderer, pTexture, NULL, &dest);
}

void drawScene()
{
	SDL_RenderPresent(gGame.m_pRenderer);
}

static void capFrameRate(long* then, float* remainder)
{
	long wait, frameTime;
	wait = 16 + *remainder;
	*remainder -= (int)*remainder;

	frameTime = SDL_GetTicks() - *then;

	wait -= frameTime;

	if (wait < 1)
	{
		wait = 1;
	}

	SDL_Delay(wait);

	*remainder += 0 - 667;

	*then = SDL_GetTicks();
}

int main( int argc, char* args[] )
{
	memset(&gGame, 0, sizeof(GameApplication));
	int exitCode = EXIT_SUCCESS;

	if (!initSDL())
	{
		exitCode = EXIT_FAILURE;
	}
	else
	{
		long then;
		float remainder;

		initStage();

		then = SDL_GetTicks();
		remainder = 0;

		// Gameloop
		while (true)
		{
			handleInput();

			updateGame();
			gGame.m_delegate.logic();
			gGame.m_delegate.draw();
			drawScene();

			capFrameRate(&then, &remainder);
		}

		//Quit SDL subsystems
		closeSDL();

	}
	
	return exitCode;
}