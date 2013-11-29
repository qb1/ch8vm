/* linux & SDL specific */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "ch8vm.h"

#define _KEY ch8_State->Key
#define PIX_SIZE 10

SDL_Surface* screen = NULL;

void ch8_OS_Init()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		printf( "SDL Init error: %s\n", SDL_GetError() );
		return;
	}

	screen = SDL_SetVideoMode( 64*PIX_SIZE, 32*PIX_SIZE, 32, SDL_SWSURFACE );
	if (screen == NULL)
	{
		printf( "SDL Create window error %s\n", SDL_GetError() );
		return;
	}	

	SDL_FillRect( screen, NULL, SDL_MapRGB( screen->format, 0, 0, 0 ) );
	
	//Update Screen
    SDL_Flip( screen );
}

int ch8_OS_tick(uint32_t *tick)
{
	// New frame begins, read events
	SDL_Event event;

	*tick = SDL_GetTicks();

	while(SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
			return 1;
            break;
        default:
            break;
        }        
    }

    return 0;
}

int ch8_OS_ReadKeys()
{
	int i, ret = -1;
	uint8_t *keystate;
	int value_per_key[0x10] =
	{
		SDLK_KP0,
		SDLK_KP7,
		SDLK_KP8,
		SDLK_KP9,
		SDLK_KP4,
		SDLK_KP5,
		SDLK_KP6,
		SDLK_KP1,
		SDLK_KP2,
		SDLK_KP3,
		SDLK_KP_PERIOD,
		SDLK_KP_ENTER,
		SDLK_KP_MULTIPLY,
		SDLK_KP_DIVIDE,
		SDLK_KP_MINUS,
		SDLK_KP_PLUS,
	};

	keystate = SDL_GetKeyState(NULL);

	for( i=0; i < sizeof(value_per_key)/sizeof(*value_per_key); ++i )
	{
		_KEY[i] = keystate[value_per_key[i]]?1:0;
		
		if( _KEY[i] )
			ret = i;
	}

	return ret;
}

void ch8_OS_PrintScreen()
{
	SDL_Rect rect;
	unsigned char* scr;

	rect.w = rect.h = PIX_SIZE;

	// ch8_printState();

	SDL_FillRect( screen, NULL, SDL_MapRGB( screen->format, 0, 0, 0 ) );

	rect.y = 0;
	for( int i=0; i < CH8_SCREEN_HEIGHT; ++i )
	{
		scr = ch8_State->Screen + i * (unsigned char)(CH8_SCREEN_WIDTH/8);
		
		rect.x = 0;
		for( int j=0; j < CH8_SCREEN_WIDTH; ++j )
		{
			if( scr[j/8] & 1<<(j%8) )
			{
				rect.x = j*PIX_SIZE;
				SDL_FillRect( screen, &rect, SDL_MapRGB( screen->format, 0, 255, 0 ) );	
			}

			rect.x += PIX_SIZE;
		}

		rect.y += PIX_SIZE;
	}

	//Update Screen
    SDL_Flip( screen );

}