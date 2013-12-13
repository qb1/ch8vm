/* linux & SDL specific */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <SDL.h>
#include <SDL_thread.h>

#include "ch8vm.h"

#define _KEY ch8_State->Key
#define PIX_SIZE 10

int ch8_OS_cputhread( void *data );
void ch8_OS_PrintScreen(int x, int y, int w, int h);

SDL_Surface *screen = NULL;
SDL_Thread  *cpu_thread = NULL;

int 		quit = 0;
SDL_Rect 	screen_dirty= {0};

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

void ch8_OS_Start()
{
	// Create the main CPU thread
    cpu_thread = SDL_CreateThread( ch8_OS_cputhread, NULL );

    SDL_WaitThread( cpu_thread, NULL );
}

void ch8_OS_Pause()
{
	ch8_State->Paused = 1;

	// Draw pause sign
	SDL_Rect rect;
	rect.w = PIX_SIZE*4;
	rect.h = PIX_SIZE*CH8_SCREEN_HEIGHT/2;
	rect.y = PIX_SIZE*(CH8_SCREEN_HEIGHT/2 - CH8_SCREEN_HEIGHT/4);

	rect.x = (CH8_SCREEN_WIDTH/2-3)*PIX_SIZE;
	SDL_FillRect( screen, &rect, SDL_MapRGB( screen->format, 128, 128, 128 ) );	
	rect.x = (CH8_SCREEN_WIDTH/2+3)*PIX_SIZE;
	SDL_FillRect( screen, &rect, SDL_MapRGB( screen->format, 128, 128, 128 ) );	

	SDL_Flip( screen );
}

void ch8_OS_Resume()
{
	ch8_State->Paused = 0;
	ch8_OS_PrintScreen( 0, 0, CH8_SCREEN_WIDTH, CH8_SCREEN_HEIGHT );
}

void ch8_OS_updatekeys( int *key )
{
	int i;
	SDL_Event event;
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

	*key = -1;
	while(SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
        	quit = 1;
        	exit(1);
            break;

        case SDL_ACTIVEEVENT:
        	if( event.active.state == SDL_APPINPUTFOCUS || event.active.state == SDL_APPACTIVE )
        	{
		    	if( event.active.gain == 0 )
		    		ch8_OS_Pause();
		    	else
		    		ch8_OS_Resume();
		    }
        	break;

        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
        	if( event.button.state == SDL_RELEASED && ch8_State->Paused )
        	{
        		ch8_OS_Resume();
        	}
        	break;


        case SDL_KEYDOWN:
        case SDL_KEYUP:
    	{
    		for( i=0; i < sizeof(value_per_key)/sizeof(*value_per_key); ++i )
			{
				if( event.key.keysym.sym == value_per_key[i] )
				{
					if( event.key.type == SDL_KEYDOWN )
					{
						_KEY[i] = 1;
						*key = i;
					}else{
						_KEY[i] = 0;
					}
				}
			}
    		break;
    	}

        default:
            break;
        }        
    }
}

uint32_t ch8_OS_timediff(const struct timespec * end, const struct timespec * start)
{
	struct timespec temp;
	uint32_t ret;

	if( (end->tv_nsec-start->tv_nsec)<0 )
	{
		temp.tv_sec = end->tv_sec-start->tv_sec-1;
		temp.tv_nsec = 1000000000+end->tv_nsec-start->tv_nsec;
	}else{
		temp.tv_sec = end->tv_sec-start->tv_sec;
		temp.tv_nsec = end->tv_nsec-start->tv_nsec;
	}

	ret = temp.tv_sec*1000000;
	ret += temp.tv_nsec / 1000;
	return ret;
}

void ch8_OS_tick( /*int *key*/ )
{
	int key;
	const uint32_t frame_time_cpu = 1000000/CH8_FPS;
    const uint32_t frame_time_60hz = 1000000/CH8_TIMER_HZ;
    const uint32_t frame_time_screen = 1000000/CH8_SCREEN_HZ;

	static struct timespec ts_last;
	static struct timespec ts_last_60hz;
	static struct timespec ts_last_screen;
    
    struct timespec ts_cur;    

    uint32_t elapsed_cpu;
    uint32_t elapsed_60hz;
    uint32_t elapsed_screen;    

    clock_gettime(CLOCK_MONOTONIC, &ts_cur);
	if( ts_last.tv_sec == 0 && ts_last.tv_nsec == 0 )	
	{
		// First frame		
		ts_last = ts_last_60hz = ts_last_screen = ts_cur;
		return;
	}

	// Start by drawing screen
	elapsed_screen = ch8_OS_timediff( &ts_cur, &ts_last_screen );
	if( elapsed_screen > frame_time_screen )
	{
		ts_last_screen = ts_cur;
		if( screen_dirty.x != 0 || screen_dirty.y != 0 || screen_dirty.w != 0 || screen_dirty.h != 0 )
		{
			ch8_OS_PrintScreen( screen_dirty.x, screen_dirty.y, screen_dirty.w, screen_dirty.h );
			screen_dirty.x = screen_dirty.y = screen_dirty.w = screen_dirty.h = 0;
		}    		
	}

	// Should we be waiting for an event?
	while( ch8_State->PausedOnKey != -1 || ch8_State->Paused )
	{
		// Reduce FPS to a minimum
		usleep( 1000000/CH8_PAUSED_FPS );
		ch8_OS_updatekeys( &key );
	}

    // Wait until next frame
    clock_gettime(CLOCK_MONOTONIC, &ts_cur);
	elapsed_cpu = ch8_OS_timediff( &ts_cur, &ts_last );
	if( elapsed_cpu < frame_time_cpu )
	{
		usleep( frame_time_cpu-elapsed_cpu );
	}	

	// New frame begins
	clock_gettime(CLOCK_MONOTONIC, &ts_last);

	// Update timers
	elapsed_60hz = ch8_OS_timediff( &ts_last, &ts_last_60hz );
	if( elapsed_60hz > frame_time_60hz )
	{
		ts_last_60hz = ts_last;
		ch8_VMTimerUpdate();
	}	

	
	ch8_OS_updatekeys( &key );

	// Instr will be called now
}

int ch8_OS_cputhread( void *data )
{
	int key=0;

	while( !quit )
    {
    	ch8_OS_tick( /*&key*/ );    	

    	// Execute VM step
    	ch8_VMStep( key );
    }

    return 0;
}

void ch8_OS_UpdateScreen(int x, int y, int w, int h)
{
	if( screen_dirty.x == 0 && screen_dirty.y == 0 && screen_dirty.w == 0 && screen_dirty.h == 0 )
    {
    	screen_dirty.x = x;
    	screen_dirty.y = y;
    	screen_dirty.w = w;
    	screen_dirty.h = h;
    	return;
    }

	if( x < screen_dirty.x  )
	{
		screen_dirty.w += screen_dirty.x-x;
		screen_dirty.x = x;
	}
	if( y < screen_dirty.y  )
	{
		screen_dirty.h += screen_dirty.y-y;
		screen_dirty.y = y;
	}
	if( x+w > screen_dirty.x+screen_dirty.w  )
	{
		screen_dirty.w = x+w-screen_dirty.x;
	}
	if( y+h > screen_dirty.y+screen_dirty.h  )
	{
		screen_dirty.h = y+h-screen_dirty.y;
	}
}

void ch8_OS_PrintScreen(int x, int y, int w, int h)
{
	SDL_Rect rect;
	unsigned char* scr;
	int i,j, wcount;

	rect.w = rect.h = PIX_SIZE;

	for( i=y; h>0; ++i, --h )
	{
		if( i >= CH8_SCREEN_HEIGHT || i < 0 )
		{
			i %= CH8_SCREEN_HEIGHT;
		}

		rect.y = i*PIX_SIZE;
		scr = ch8_State->Screen + i * (unsigned char)(CH8_SCREEN_WIDTH/8);
		
		for( j=x, wcount=w; wcount>0; ++j,--wcount )
		{
			if( j >= CH8_SCREEN_WIDTH || j < 0 )
			{
				j %= CH8_SCREEN_WIDTH;
			}

			rect.x = j*PIX_SIZE;
			if( scr[j/8] & 1<<(j%8) )
			{				
				SDL_FillRect( screen, &rect, SDL_MapRGB( screen->format, 0, 255, 0 ) );	
			}else{
				SDL_FillRect( screen, &rect, SDL_MapRGB( screen->format, 0, 0, 0 ) );	
			}
		}
	}

	//Update Screen
    SDL_Flip( screen );
}