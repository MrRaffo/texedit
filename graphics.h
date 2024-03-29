/*
    graphics.h
    a software graphics library using 2 banks of screen memory for double buffering.
    all drawing is done to raw memory which are then transferred to a SDL_Surface of the same
    res, which is then upscaled to another SDL_Surface to allow any window size to show any
    lower resolution - ie a 320x200 resolution in a 1024x640 window.

    made for use with my texture and palette definitions to give an old fashioned look
*/

#ifndef __graphics_h__
#define __graphics_h__


//===============================================================
//  DEFINE
//===============================================================

// error reporting
#define GRA_Print_SDL_Error()           printf( "SDL Error: %s\n", SDL_GetError() )

// all color data will be of type uint32_t, so these values are used to edit colours
#if     SDL_BYTEORDER == SDL_BIG_ENDIAN
    // colour masks
    #define     R_MASK              0xff000000
    #define     G_MASK              0x00ff0000
    #define     B_MASK              0x0000ff00
    #define     A_MASK              0x000000ff

    // colour adjustments - multiply a uint8_t by these to give correct placement in uint32_t
    #define     R_ADJUST            0x1000000
    #define     G_ADJUST            0x10000
    #define     B_ADJUST            0x100
    #define     A_ADJUST            0x1

#else
    
    #define     R_MASK              0x000000ff
    #define     G_MASK              0x0000ff00
    #define     B_MASK              0x00ff0000
    #define     A_MASK              0xff000000

    #define     R_ADJUST            0x1
    #define     G_ADJUST            0x100
    #define     B_ADJUST            0x10000
    #define     A_ADJUST            0x1000000
#endif  // SDL_BYTEORDER

//===============================================================
//  STRUCTS AND TYPES
//===============================================================

struct scr_buffer_s             {
                                    int         w;
                                    int         h;

                                    uint32_t    *buffer1;
                                    uint32_t    *buffer2;
                                };
typedef struct scr_buffer_s scr_buffer_type;


// TODO
//struct  texture_s               {};

//===============================================================
//  FUNCTION PROTOTYPES
//===============================================================

// All int returning functions return 1 on success or 0 on failure unless otherwise stated

//=======================
//  INITIALIZATION
//=======================

// starts SDL Video and opens a window. Also creates a screen_buffer_type object for 
// writing to and initializes 2 SDL_Surfaces - one w_res x h_res which is stretched onto the
// second which is width x height, which is rendered to the window.
int GRA_Create_Display( char *title, int width, int height, int w_res, int h_res );


// frees the SDL types, such as the window and surfaces and the 
void GRA_Close();


//=======================
//  CONTROL
//=======================

// wrapper for SDL_Delay, stalls program for milli milliseconds
void GRA_Delay( int milli );

// check if user quits, by clicking window 'x' or pressed escape
int GRA_Check_Quit();



//=======================
//  GRAPHICS
//=======================

// clears the current buffer for writing
void GRA_Clear_Screen();


// fill current buffer with a color from the palette
void GRA_Fill_Screen( int color_index );


// writes the current active buffer to the render_surface and displays it, then
// switches buffers for the next write
void GRA_Refresh_Window();


// generates a 256 colour palette
int GRA_Generate_Palette();


// loads a 256 colour palette from file
int GRA_Load_Palette( char *filename );



// returns corresponding uint32_t for r g b a colour
uint32_t GRA_Create_Color( uint8_t r, uint8_t g, uint8_t b, uint8_t a );


// returns color at given palette index
uint32_t GRA_Get_Palette_Color( int index );


// draws a pixel at given coordinates, uses color variable as an index for the palette table,
// not as a RGBA value to draw
void GRA_Set_Palette_Pixel( int x, int y, int color_index );


// draws a pixel at the given coordinates, using color as a RGBA value
void GRA_Set_RGBA_Pixel( int x, int y, uint32_t color_rgba );

// draws a vertical line of given color index to the buffer, uses color as palette index
void GRA_Draw_Vertical_Line( int x, int y1, int y2, uint32_t color_rgba );


// draws a textured column of pixels to the screen      - TODO
void GRA_Draw_Vertical_Texture_Line();


// draws a horizontal line
void GRA_Draw_Horizontal_Line( int x1, int x2, int y, uint32_t color_rgba );


// draws a hollow rectangle to the screen
void GRA_Draw_Hollow_Rectangle( int x, int y, int w, int h, uint32_t color_rgba );


// draws a filled rectangle to the screen
void GRA_Draw_Filled_Rectangle( int x, int y, int w, int h, uint32_t color_rgba );



//==========================
//  TEXTURES
//==========================


// loads textures from file
int GRA_Load_Texture();     // TODO



//==========================
//  TEXT
//==========================


// loads my own custom made font files for use in these functions - TODO
int GRA_Load_Font( char *filename );


// places a character at (x, y), draw_bg is a flag used to tell whether or not to draw the 
// background color
void GRA_Place_Char( int letter, int x, int y, int forecolor, int bgcolor, int draw_bg );


// writes a string of text to the buffer, does not check for boundaries and does not wrap text
void GRA_Simple_Text( char *str, int x, int y, int forecolor, int bgcolor, int draw_bg );


//==========================
//  CONTROL
//==========================

// wrapper for SDL_GetMouseState, returns 1 for LMB pressed, 2 for RMB pressed
uint32_t GRA_Get_Mouse_State( int *x, int *y );


//===========================
//  TESTING
//===========================


//===============================================================
//  FUNCTION BODIES
//===============================================================

#endif  // __ graphics_h__
