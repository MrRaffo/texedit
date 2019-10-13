/*
    graphics.h
    a software graphics library using 2 banks of screen memory for double buffering.
    all drawing is done to raw memory which are then transferred to a SDL_Surface of the same
    res, which is then upscaled to another SDL_Surface to allow any window size to show any
    lower resolution - ie a 320x200 resolution in a 1024x640 window.

    made for use with my texture and palette definitions to give an old fashioned look
*/

#include <stdio.h>
#include <stdint.h>

#include <SDL2/SDL.h>

#include "utility.h"
#include "graphics.h"


//===============================================================
//  CONSTANTS AND GLOBALS
//===============================================================

#define PALETTE_SIZE            256

//====================
//  DISPLAY
//====================

static SDL_Window           *scr_window         = NULL;         // display window
static SDL_Surface          *scr_surface        = NULL;         // window surface
static SDL_Surface          *scr_render         = NULL;         // render surface

static SDL_Rect             scr_rect           = { 0, 0, 0, 0 };

static uint32_t             *w_buffer           = NULL;         // buffer to write to
static uint32_t             *r_buffer           = NULL;         // buffer to display (read from)

static int                  scr_width           = 0;            // window dimensions
static int                  scr_height          = 0;            

static int                  res_width           = 0;            // render dimensions
static int                  res_height          = 0;

// double buffer to write to
static scr_buffer_type      scr_buffer          = { 0, 0, NULL, NULL };

static uint32_t             *palette            = NULL;

// TODO
// static texture_type      **textures          = NULL;

// font data is loaded here
static uint8_t             *font_buffer        = NULL;

//===============================================================
//  PRIVATE FUNCTIONS
//===============================================================

// All int returning functions return 1 on success or 0 on failure unless otherwise stated

// draws the w_buffer to the render surface
void Draw_Buffer()
{
    uint32_t *bufp;
    bufp = scr_render->pixels;              // get pointer to surface pixel data
    int x, y;

    for( y = 0; y < scr_render->h; y++ )
    {
        for( x = 0; x < scr_render->w; x++ )
        {
            *bufp = w_buffer[y * scr_render->w + x];
            bufp++;
        }
    }

    return;
}


// swaps the pointers to w_buffer and r_buffer
void Swap_Buffer()
{
    uint32_t *temp;
    temp = w_buffer;
    w_buffer = r_buffer;
    r_buffer = temp;

    return;
}



//===============================================================
//  FUNCTION BODIES
//===============================================================

// All int returning functions return 1 on success or 0 on failure unless otherwise stated

//=======================
//  INITIALIZATION
//=======================

// starts SDL Video and opens a window. Also creates a screen_buffer_type object for 
// writing to and initializes 2 SDL_Surfaces - one w_res x h_res which is stretched onto the
// second which is width x height, which is rendered to the window.
int GRA_Create_Display( char *title, int width, int height, int w_res, int h_res )
{
 
    // initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO ) != 0 )
    {
        UTI_Print_Error( "Unable to initialize SDL" );
        GRA_Print_SDL_Error();
        return 0;
    }

    
    // set screen globals
    scr_width = width;
    scr_height = height;

    res_width = w_res;
    res_height = h_res;


    // create display window
    scr_window = SDL_CreateWindow(  title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                    width, height, SDL_WINDOW_SHOWN );
    if( scr_window == NULL )
    {
        UTI_Print_Error( "Unable to create display window" );
        GRA_Print_SDL_Error();
        return 0;
    }

    // create display surface
    scr_surface = SDL_GetWindowSurface( scr_window );
    if( scr_surface == NULL )
    {
        UTI_Print_Error( "Unable to get window surface" );
        GRA_Print_SDL_Error();
        return 0;
    }


    // create render surface
    scr_render = SDL_CreateRGBSurface(  SDL_SWSURFACE, w_res, h_res, 32,
                                        R_MASK, G_MASK, B_MASK, A_MASK );
    if( scr_render == NULL )
    {
        UTI_Print_Error( "Unable to create render surface" );
        GRA_Print_SDL_Error();
        return 0;
    }

    // create rect for blitting render to screen
    scr_rect.x = 0;
    scr_rect.y = 0;
    scr_rect.w = width;
    scr_rect.h = height;

    // create double buffer
    scr_buffer.w = w_res;
    scr_buffer.h = h_res;
    scr_buffer.buffer1 = UTI_EC_Malloc( sizeof( uint32_t ) * w_res * h_res );
    scr_buffer.buffer2 = UTI_EC_Malloc( sizeof( uint32_t ) * w_res * h_res );

    // set write and read buffer pointers
    w_buffer = scr_buffer.buffer1;
    r_buffer = scr_buffer.buffer2;


    return 1;
}


// frees the SDL types, such as the window and surfaces and the 
void GRA_Close()
{
    // free SDL_ Structs
    SDL_DestroyWindow( scr_window );
    scr_window = NULL;

    SDL_FreeSurface( scr_render );
    scr_render = NULL;

    // free screen buffers
    UTI_EC_Free( scr_buffer.buffer1 );
    scr_buffer.buffer1 = NULL;

    UTI_EC_Free( scr_buffer.buffer2 );
    scr_buffer.buffer2 = NULL;

    // free font data
    UTI_EC_Free( font_buffer );
    
    // TODO - free texture data
    

    return;
}


//=======================
//  CONTROL
//=======================

// wrapper for SDL_Delay, stalls program for milli milliseconds
void GRA_Delay( int milli )
{
    SDL_Delay( milli );
    return;
}

// check if user quits, by clicking window 'x' or pressed escape
int GRA_Check_Quit()
{
    SDL_Event e;

    while( SDL_PollEvent( &e ) != 0 )
    {
        // check for user closing window
        if( e.type == SDL_QUIT )
        {
            return 0;
        }
        else if( e.type == SDL_KEYDOWN )
        {
            // check for user pressing escape
            switch( e.key.keysym.sym )
            {
                case SDLK_ESCAPE:
                    return 0;
                    break;

                default:
                    break;
            }
        }
    }

    return 1;
}



//=======================
//  GRAPHICS
//=======================

// clears the current buffer for writing
void GRA_Clear_Screen()
{
    int i;
    for( i = 0; i < res_width * res_height; i++ )
    {
        w_buffer[i] = 0;        // black
    }

    return;
}

// TODO
void GRA_Fill_Screen( int color_index )
{
    return;
}



// writes the current active buffer to the render_surface and displays it, then
// switches buffers for the next write
void GRA_Refresh_Window()
{
    Draw_Buffer();
    Swap_Buffer();

    SDL_BlitScaled( scr_render, NULL, scr_surface, &scr_rect );

    SDL_UpdateWindowSurface( scr_window );

    return;
}



// generates a 256 colour palette
int GRA_Generate_Palette()
{

    palette = UTI_EC_Malloc( sizeof( uint32_t ) * PALETTE_SIZE );

    int r, g, b, a = 0xff;
    for( r = 0; r < 8; r++ )
    {
        for( g = 0; g < 8; g++ )
        {
            for( b = 0; b < 4; b++ )
            {
                palette[r*32+g*4+b] = GRA_Create_Color( r*32, g*32, b*64, a );
            }
        }
    }

    return 1;
}



// loads a 256 colour palette from file
int GRA_Load_Palette( char *filename )
{

    return 1;
}



// returns corresponding uint32_t for r g b a colour
uint32_t GRA_Create_Color( uint8_t r, uint8_t g, uint8_t b, uint8_t a )
{
    return( r*R_ADJUST + g*G_ADJUST + b*B_ADJUST + a*A_ADJUST );
}



// returns color at given palette index
uint32_t GRA_Get_Palette_Color( int index )
{
    if( index < 0 || index > PALETTE_SIZE )
    {
        return 0;
    }

    return palette[index];
}

// draws a pixel at given coordinates, uses color variable as an index for the palette table,
// not as a RGBA value to draw
void GRA_Set_Palette_Pixel( int x, int y, int color )
{ 
    printf( "%x\n", palette[color] );
    GRA_Set_RGBA_Pixel( x, y, palette[color] );

    return;
}


// draws a pixel at the given coordinates, using color as RGBA value
void GRA_Set_RGBA_Pixel( int x, int y, uint32_t color )
{
    // check if pixel is within screen bounds
    if( x < 0 || x > res_width || y < 0 || y > res_height )
    {
        return;
    }

    w_buffer[y*res_width + x] = color;

    return;
}


// draws a vertical line of given color index to the buffer, uses color as RGBA value
void GRA_Draw_Vertical_Line( int x, int y1, int y2, uint32_t color )
{
    // check line is within screen bounds
    if( x < 0 || x >= res_width )
    {
        return;
    }

    // make sure y2 is larger (lower on screen)
    if( y1 > y2 )
    {
        int temp = y1;
        y1 = y2;
        y2 = temp;
    }

    // make sure line is within vertical screen bounds
    if( y2 < 0 || y1 > res_height )
    {
        return;
    }

    // if line is on screen but extremes are off, set extremes within screen limits
    if( y1 < 0 )            y1 = 0;
    if( y2 > res_height-1)  y2 = res_height-1;

    // draw the line
    for( ; y1 <= y2; y1++ )
    {
        GRA_Set_RGBA_Pixel( x, y1, color );
    }

    return;
}


// draws a textured column of pixels to the screen      - TODO need texture
void GRA_Draw_Vertical_Texture_Line(){ return; }        


// draws a horizontal line
void GRA_Draw_Horizontal_Line( int x1, int x2, int y, uint32_t color )
{
    // check line is on screen
    if( y < 0 || y >= res_height )
    {
        return;
    }

    // check x1 is lower number
    if( x1 > x2 )
    {
        int temp = x1;
        x1 = x2;
        x2 = temp;
    }

    // check line is within horizontal screen bounds
    if( x2 < 0 || x1 > res_width )
    {
        return;
    }

    // if line extremes are off-screen, adjust them
    if( x1 < 0 )                x1 = 0;
    if( x2 > res_width-1 )      x2 = res_width-1;

    // draw the line
    for( ; x1 <= x2; x1++ )
    {
        GRA_Set_RGBA_Pixel( x1, y, color );
    }

    return;
}


// draws a hollow rectangle to the screen
void GRA_Draw_Hollow_Rectangle( int x, int y, int w, int h, uint32_t color )
{
    GRA_Draw_Vertical_Line( x,      y,      y+h,    color );
    GRA_Draw_Vertical_Line( x+w,    y,      y+h,    color );

    GRA_Draw_Horizontal_Line( x,    x+w,    y,      color );
    GRA_Draw_Horizontal_Line( x,    x+w,    y+h,    color );

    return;
}


// draws a filled rectangle to the screen - TODO - cant use until palette implemented
void GRA_Draw_Filled_Rectangle( int x, int y, int w, int h, uint32_t color )
{
    int i;
    for( i = 0; i < w; i++ )
    {
        GRA_Draw_Vertical_Line( x+i, y, y+h, color );
    }

    return;
}

//==========================
//  TEXTURES
//==========================


// loads textures from file
int GRA_Load_Texture(){ return 1; }     // TODO



//==========================
//  TEXT
//==========================

#include <ctype.h>
#include <string.h>

// size in bytes of a font file
#define FONT_FILE_SIZE          2048

#define CHAR_SET_SIZE           256     // no of chars in set
#define CHAR_SIZE               8       // size in bytes

#define CHAR_WIDTH              8       // in pixels
#define CHAR_HEIGHT             8

// loads my own custom made font files for use in these functions - TODO
int GRA_Load_Font( char *filename )
{ 
    // open file for reading
    FILE    *file = NULL;
    int     filesize = 0;

    file = fopen( filename, "r" );
    if( file == NULL )
    {
        UTI_Print_Error( "Unable to open font file\n" );
        return 0;
    }

    // check filesize is correct
    fseek( file, 0, SEEK_END );
    filesize = ftell( file );
    rewind( file );
    
    if( filesize != FONT_FILE_SIZE )
    {
        UTI_Print_Error( "Font file is incorrect size" );
        return 0;
    }

    // create temp buffer
    uint8_t *temp_buffer;
    temp_buffer = UTI_EC_Malloc( filesize );

    // load data to buffer
    fread( temp_buffer, filesize, 1, file );
    
    // create font buffer
    font_buffer = UTI_EC_Malloc( filesize * CHAR_SIZE );

    // unpack font data
    int i, j;
    uint8_t c;
    for( i = 0; i < filesize; i++ )
    {
        for( j = 0; j < CHAR_SIZE; j++ )
        {
            c = temp_buffer[i] & (0x80 >> j);
            font_buffer[i*CHAR_SIZE + j] = (c) ? 1 : 0;
        }
    }

    UTI_EC_Free( temp_buffer );
    fclose( file );
    
    return 1; 
}


// places a character at (x, y), draw_bg is a flag used to tell whether or not to draw the 
// background color
void GRA_Place_Char( int letter, int x, int y, int forecolor, int bgcolor, int draw_bg )
{
    int offset = letter * CHAR_WIDTH * CHAR_HEIGHT; // this can be changed for non-ascii sets
    int i, j, pixel;

    for( i = 0; i < CHAR_HEIGHT; i++ )
    {
        for( j = 0; j < CHAR_WIDTH; j++ )
        {
            pixel = font_buffer[i*CHAR_WIDTH+j+offset];
            // check if pixel should be drawn (always if draw_bg is set)
            if( draw_bg == 0 && pixel == 0 )
            {
                continue;
            }
            else
            {
                GRA_Set_RGBA_Pixel( x+j, y+i, (pixel) ? forecolor : bgcolor );
            }
        }
    }

    return;
}


// writes a string of text to the buffer, does not check for boundaries and does not wrap text
void GRA_Simple_Text( char *str, int x, int y, int forecolor, int bgcolor, int draw_bg )
{
    int i, len = strlen( str ) + 1;

    for( i = 0; i < len; i++ )
    {
        GRA_Place_Char( str[i], x, y, forecolor, bgcolor, draw_bg );
        x += CHAR_WIDTH;
    }

    return;
}


//==========================
//  CONTROL
//==========================

// wrapper for SDL_GetMouseState, returns 1 for LMB pressed, 2 for RMB pressed
uint32_t GRA_Get_Mouse_State( int *x, int *y )
{
    uint32_t state = SDL_GetMouseState( x, y );
    
    if( state & SDL_BUTTON( SDL_BUTTON_LEFT ) )
    {
        return 1;
    }
    else if( state & SDL_BUTTON( SDL_BUTTON_RIGHT ) )
    {
        return 2;
    }

    return 0;
}


//===========================
//  TESTING
//===========================


