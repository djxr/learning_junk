/*
 *	Minimum Viable Product
 *	
 *	*add a ping, whenever damaged or pickup SDL_RenderFillRect(rend, SCREEN_RECT)
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <SDL2/SDL.h>

#define TILES_WIDE	6
#define	TILES_HIGH	1
#define	TILE_SIZE	16
#define	scale		64
#define DEMO_ENEMIES	8

typedef unsigned char byte; // Holds at least values 0-255.
typedef unsigned short dbyte; // Holds at least values 0-65,535.

typedef enum sprite_index /*{{{*/
{
	crosshair,
	guy,
	demon,
	block,
	plus,
	shell_pile
	
}sprite_index; // if memory bloat becomes a problem this can be optimized /*}}}*/
typedef enum identifier/*{{{*/
{
	human,
	ai,
	health_plus,
	shells_plus

}identifier;/*}}}*/

typedef struct npc /*{{{*/
{
	byte		health;
	SDL_Rect	rect;
	sprite_index	sprite; // TO OPTIMIZE: change to byte or (dbyte if more than 256 sprites)
	bool		hit;
	bool		up;
	bool		left;
	bool		right;
	bool		down;
}npc;/*}}}*/
typedef struct item/*{{{*/
{
	byte		magnitude;
	SDL_Rect	rect;
	sprite_index	sprite; // TO OPTIMIZE: change to byte or (dbyte if more than 256 sprites)
}item;/*}}}*/
typedef struct tile_atlas/*{{{*/
{
	SDL_Texture	*tex;
	SDL_Rect	tile[TILES_WIDE];
}tile_atlas;/*}}}*/

// struct hero player; /*{{{*/
struct hero 
{
	byte		health;
	byte		immune;
	SDL_Rect	rect;
	sprite_index	sprite;
	bool		firing;
	bool		up;
	bool		left;
	bool		right;
	bool		down;
	byte		shells;
}player;/*}}}*/
// struct cursor mouse; /*{{{*/
struct cursor
{
	SDL_Rect	rect;
	sprite_index	sprite;
}mouse;/*}}}*/

// Global
int		i; // ONLY USE IN FOR LOOPS!
int		j; // ONLY USE IN FOR LOOPS!
byte		hs = scale / 2;
bool		play =	true;
SDL_Window	*win =	NULL;
SDL_Renderer	*rend =	NULL;
npc		monster[DEMO_ENEMIES];
item		hpack;
item		shpack;
tile_atlas	atlas;
SDL_Rect	disp; // TODO: Update this on resize
SDL_Point	gridsize; // TODO: Update this on resize
// Config
Uint32		UPS =	120;
Uint32		FPS =	60;

int init()/*{{{*/
{
	srand((unsigned int)1000);
	SDL_DisplayMode	dm;

	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_EVENTS))
	{
		fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
		return 1;
	}
	SDL_Log("I initialized SDL");
	if(SDL_GetCurrentDisplayMode(1, &dm))
	{
		SDL_Log("SDL_GetCurrentDisplayMode() failed: %s", SDL_GetError());
		return 1;
	}
	SDL_Log("Display Mode properties are h:%d w:%d refresh_rate:%d", dm.h, dm.w, dm.refresh_rate);
	if(SDL_GetDisplayBounds(1, &disp))
	{
		SDL_Log("SDL_GetDisplayBounds failed: %s",SDL_GetError());
		return 1;
	}
	SDL_Log("Display Bounds properties are x:%d y:%d h:%d w:%d", disp.x, disp.y, disp.h, disp.w);
	win = SDL_CreateWindow("ONE", disp.x, disp.y, disp.w, disp.h, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
	if(win == NULL)
	{
		SDL_Log("SDL_CreateWindow() Failed: %s", SDL_GetError());
		return 1;
	}
	rend = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	if(rend == NULL)
	{
		SDL_Log("SDL_CreateRenderer() Failed: %s", SDL_GetError());
		return 1;
	}
	SDL_ShowCursor(0);

	return 0;
}/*}}}*/
int tini()/*{{{*/
{
	SDL_DestroyTexture(atlas.tex);
	SDL_DestroyRenderer(rend);
	SDL_DestroyWindow(win);
	SDL_Quit();
	return 0;
}/*}}}*/
int init_atlas()/*{{{*/
{
	SDL_Surface *sur = SDL_LoadBMP("tilestwo.bmp");
	SDL_SetColorKey(sur, SDL_TRUE, SDL_MapRGB(sur->format, 0, 255, 255));
	atlas.tex = SDL_CreateTextureFromSurface(rend, sur);
	SDL_FreeSurface(sur);
	// atlas.tile[] assignment 
	for(i = 0; i < TILES_WIDE; i++)
	{
		atlas.tile[i].x = i * TILE_SIZE;
		atlas.tile[i].y = 0 * TILE_SIZE;
		atlas.tile[i].w = TILE_SIZE;
		atlas.tile[i].h = TILE_SIZE;
	}
	return 0;
}/*}}}*/

int handle_events(SDL_Event *e)/*{{{*/
{
	switch(e->type)
	{
		case SDL_QUIT:
			play = false;
			break;
		case SDL_MOUSEMOTION:
			mouse.rect.x = e->motion.x;
			mouse.rect.y = e->motion.y;
			break;
		case SDL_MOUSEWHEEL:
			break;
		case SDL_MOUSEBUTTONDOWN:
			switch(e->button.button)
			{
				case SDL_BUTTON_LEFT:
					if(player.shells)
					{
						for(i = 0; i < DEMO_ENEMIES; i++)
						{
							if(	monster[i].rect.x	< e->button.x &&
								e->button.x	< monster[i].rect.x + scale &&
								monster[i].rect.y	< e->button.y &&
								e->button.y	< monster[i].rect.y + scale)
							{
								monster[i].hit = true; // MOVE TO LOGIC?
							}
						}
						player.firing = true;
						player.shells--;
					}
					break;
			}
			break;
		case SDL_KEYDOWN:
			switch(e->key.keysym.sym)
			{
				case SDLK_ESCAPE:
					play = false;
					break;
				case SDLK_w:
					player.up = true;
					break;
				case SDLK_a:
					player.left = true;
					break;
				case SDLK_d:
					player.right = true;
					break;
				case SDLK_s:
					player.down = true;
					break;
			}
			break;
		case SDL_KEYUP:
			switch(e->key.keysym.sym)
			{
				case SDLK_w:
					player.up = false;
					break;
				case SDLK_a:
					player.left = false;
					break;
				case SDLK_d:
					player.right = false;
					break;
				case SDLK_s:
					player.down = false;
					break;
			}
			break;
			
	}
	//SDL_Log("I Handled Events");
	return 0;
}/*}}}*/
int process_logic()/*{{{*/
{
	// PLAYER
	if(player.immune)
		player.immune--;
	for(i = 0; i < DEMO_ENEMIES; i++)
	{
		if(!player.immune && sqrtf(powf(player.rect.x - monster[i].rect.x, 2) +
					powf(player.rect.y - monster[i].rect.y, 2)) < scale)
		{
			if(player.health)
			{
				player.health--;
				player.immune = 30;
			}
			else
			{
				player.immune = 60;
				player.rect.x = 3 * scale;
				player.rect.y = 0;
				player.health = 3;
			}
		}
	}
	if(sqrtf(powf(player.rect.x - hpack.rect.x, 2) +
				powf(player.rect.y - hpack.rect.y, 2)) < scale)
	{
		if(player.health < 6)
		{
			hpack.rect.x = (rand() % gridsize.x) * scale;
			hpack.rect.y = (rand() % gridsize.y) * scale;
			player.health++;
		}
	}
	if(sqrtf(powf(player.rect.x - shpack.rect.x, 2) +
				powf(player.rect.y - shpack.rect.y, 2)) < scale)
	{
		if(player.shells < 12)
		{
			shpack.rect.x = (rand() % gridsize.x) * scale;
			shpack.rect.y = (rand() % gridsize.y) * scale;
			for(i = 0; i < 6; i++)
				if(player.shells < 12)
					player.shells ++;
		}
	}
	if(player.up)
		if(player.rect.y > 0)
			player.rect.y -= hs / 4;
	if(player.left)
		if(player.rect.x > 0)
			player.rect.x -= hs / 4;
	if(player.right)
		if(player.rect.x < disp.w - scale - hs)
			player.rect.x += hs / 4;
	if(player.down)
		if(player.rect.y < disp.h - scale - hs)
			player.rect.y += hs / 4;

	// MONSTERS
	for(i = 0; i < DEMO_ENEMIES; i++)
	{
		if(monster[i].hit)/*{{{*/
		{
			monster[i].hit = false;
			switch(rand() % 4)
			{
				case 0:
					monster[i].rect.x = (rand() % gridsize.x) * scale;
					monster[i].rect.y = -1 * scale;
					break;
				case 1:
					monster[i].rect.x = -1 * scale;
					monster[i].rect.y = (rand() % gridsize.y) * scale;
					break;
				case 2:
					monster[i].rect.x = (rand() % gridsize.x) * scale;
					monster[i].rect.y = gridsize.y * scale;
					break;
				case 3:
					monster[i].rect.x = gridsize.x * scale;
					monster[i].rect.y = (rand() % gridsize.y) * scale;
					break;
			}
		}/*}}}*/

		// Movement /*{{{*/
		if(monster[i].rect.y > player.rect.y)
			monster[i].rect.y -= hs / 8;
		if(monster[i].rect.x > player.rect.x)
			monster[i].rect.x -= hs / 8;
		if(monster[i].rect.x < player.rect.x)
			monster[i].rect.x += hs / 8;
		if(monster[i].rect.y < player.rect.y)
			monster[i].rect.y += hs / 8;
		monster[i].rect.x += ((rand() % 10) - 5);
		monster[i].rect.y += ((rand() % 10) - 5);
		/*}}}*/
	}

	//SDL_Log("I Processed Logic");
	return 0;
}/*}}}*/
int render(SDL_Renderer *r)/*{{{*/
{
	SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
	SDL_RenderClear(r);
	for(i = 0; i < (disp.w / scale); i++)
		for(j = 0; j < (disp.h / scale); j++)
		{
			SDL_Rect gridtile = {.x = i * scale, .y = j * scale, .w = scale, .h = scale};
			SDL_RenderCopy(r, atlas.tex, &atlas.tile[3], &gridtile);
		}

	SDL_RenderCopy(r, atlas.tex, &atlas.tile[hpack.sprite], &hpack.rect);
	SDL_RenderCopy(r, atlas.tex, &atlas.tile[shpack.sprite], &shpack.rect);

	SDL_RenderCopy(r, atlas.tex, &atlas.tile[player.sprite], &player.rect);
	for(i = 0; i < DEMO_ENEMIES; i++)
		SDL_RenderCopy(r, atlas.tex, &atlas.tile[monster[i].sprite], &monster[i].rect);
	if(player.firing)
	{
		SDL_SetRenderDrawColor(r, 255, 0, 0, 255);
		SDL_RenderDrawLine(r,
				player.rect.x + hs,
				player.rect.y + hs,
				mouse.rect.x,
				mouse.rect.y);
		SDL_RenderDrawLine(r,
				player.rect.x + hs,
				player.rect.y + hs,
				mouse.rect.x,
				mouse.rect.y + 8);
		SDL_RenderDrawLine(r,
				player.rect.x + hs,
				player.rect.y + hs,
				mouse.rect.x + 8,
				mouse.rect.y);
		SDL_RenderDrawLine(r,
				player.rect.x + hs,
				player.rect.y + hs,
				mouse.rect.x,
				mouse.rect.y - 8);
		SDL_RenderDrawLine(r,
				player.rect.x + hs,
				player.rect.y + hs,
				mouse.rect.x - 8,
				mouse.rect.y);
		player.firing = false;
	}

	for(i = 0; i < player.health; i++)
	{
		SDL_Rect lifehud = { .x = i * hs, .y = 0, .w = hs, .h = hs};
		SDL_RenderCopy(r, atlas.tex, &atlas.tile[player.sprite], &lifehud);
	}
	for(i = 0; i < player.shells; i++)
	{
		SDL_Rect lifehud = { .x = (i + 7) * hs, .y = 0, .w = hs, .h = hs};
		SDL_RenderCopy(r, atlas.tex, &atlas.tile[shell_pile], &lifehud);
	}

	mouse.rect.x -= hs;
	mouse.rect.y -= hs;
	SDL_RenderCopy(r, atlas.tex, &atlas.tile[mouse.sprite], &mouse.rect);
	mouse.rect.x += hs;
	mouse.rect.y += hs;

	SDL_RenderPresent(r);
	//SDL_Log("I Rendered");
	return 0;
}/*}}}*/

/*
 *	Keep main small
 *
 *
 */
int main(int argc, char *argv[])
{

	// Init 
	if(init())/*{{{*/
	{
		fprintf(stderr, "init() failed");
		return 1;
	}/*}}}*/
	init_atlas();
	gridsize.x = disp.w / scale;
	gridsize.y = disp.h / scale;

	// Loop Variables 
	SDL_Event	event;
	Uint32		time = 0;
	Uint32		last_render = 0;
	Uint32		last_update = 0;
	// Beings Setup TODO: do this better /*{{{*/
	player.health = 3;
	player.shells = 6;
	player.sprite = guy;
	player.rect.x = 3 * scale;
	player.rect.y = 3 * scale;
	player.rect.w = scale;
	player.rect.h = scale;
	player.firing = false;

	for(i = 0; i < DEMO_ENEMIES; i++)
	{
		monster[i].sprite = demon;
		monster[i].rect.x = (rand() % gridsize.x) * scale;
		monster[i].rect.y = (rand() % gridsize.y) * scale;
		monster[i].rect.w = scale;
		monster[i].rect.h = scale;
	}

	mouse.sprite = crosshair;
	mouse.rect.x = 0;
	mouse.rect.y = 0;
	mouse.rect.w = scale;
	mouse.rect.h = scale;

	hpack.sprite = plus;
	hpack.rect.x = (rand() % gridsize.x) * scale;
	hpack.rect.y = (rand() % gridsize.y) * scale;
	hpack.rect.w = scale;
	hpack.rect.h = scale;

	shpack.sprite = shell_pile;
	shpack.rect.x = (rand() % gridsize.x) * scale;
	shpack.rect.y = (rand() % gridsize.y) * scale;
	shpack.rect.w = scale;
	shpack.rect.h = scale;
/*}}}*/

	while(play)
	{
		time = SDL_GetTicks();

		while(SDL_PollEvent(&event))
			handle_events(&event);

		if(time >= (last_update + 8))
		{
			process_logic();
			last_update = time;
		}

		if(time >= (last_render + 16))
		{
			render(rend);
			last_render = time;
		}
	}

	if(tini())/*{{{*/
	{
		fprintf(stderr, "tini() failed");
		return 1;
	}/*}}}*/

	// Curiousities
	printf("%ld\n", sizeof(SDL_Point));

	return 0;
}
