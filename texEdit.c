/*
    texEdit.c

    a program for editing textures
*/

#include <stdio.h>
#include <stdint.h>
#include <math.h>       //for 'floor()'

#include "graphics.h"
#include "utility.h"

//====================================================================
//  DEFINES AND GLOBALS
//====================================================================

#define SCREEN_WIDTH            1440
#define SCREEN_HEIGHT           900

#define RES_WIDTH               640
#define RES_HEIGHT              400

#define SCREEN_FACTOR_X         2.25f
#define SCREEN_FACTOR_Y         2.25f

#define MAX_TEX_WIDTH           256
#define MAX_TEX_HEIGHT          256

#define TXR_EDIT_X              32
#define TXR_EDIT_Y              32
#define TXR_EDIT_W              256
#define TXR_EDIT_H              256

#define PAL_AREA_X              352
#define PAL_AREA_Y              32
#define PAL_AREA_W              256
#define PAL_AREA_H              256
#define ERASE_COLOR_X           TXR_EDIT_X
#define ERASE_COLOR_Y           SELECTED_COLOR_Y + SELECTED_COLOR_H + 8
#define ERASE_COLOR_W           SELECTED_COLOR_W
#define ERASE_COLOR_H           SELECTED_COLOR_H

#define TXR_SELECT_W            24
#define TXR_SELECT_H            12

#define TXR_SELECT_LEFT_X       TXR_EDIT_X
#define TXR_SELECT_LEFT_Y       TXR_EDIT_Y + TXR_EDIT_H + 4

#define TXR_SELECT_RIGHT_X      TXR_EDIT_X + TXR_EDIT_W - TXR_SELECT_W
#define TXR_SELECT_RIGHT_Y      TXR_SELECT_LEFT_Y

#define SELECTED_COLOR_X        TXR_EDIT_X
#define SELECTED_COLOR_Y        TXR_SELECT_LEFT_Y + TXR_SELECT_H + 8
#define SELECTED_COLOR_W        32
#define SELECTED_COLOR_H        32


static uint32_t                 selected_color = 0;
static uint32_t                 erase_color = 0;

// texture to edit
static uint32_t                 *current_texture = NULL;

static int                      TEX_SIZE = 0;              // current texture dimensions in pixels (TEX_SIZE x TEX_SIZE) TODO - load this from file or command line
static float                    PIXEL_SIZE = 0;             // how many pixels in the edit window make up one pixel on the texture

static int                      mouse_locked = 0;
static int                      mouse_timer = 0;

static char                     *filename = NULL;

//====================================================================
//  FUNCTION PROTOTYPES
//====================================================================

//=================
//  TEXTURES
//=================

// save textures to file
int Save_Textures();

// load textures from file
int Load_Textures();

// sets all pointer textures to NULL
void Init_Textures();

// create a new texture
int Generate_Texture();

// for initialization
void Get_Current_Texture();

// get next texture for editing
int Get_Next_Texture();

// get previous texture
int Get_Prev_Texture();

// draw current texture to the editing window
void Draw_Current_Texture();

// free texture memory
void Free_Textures();

//==================
//  GUI
//==================

// draws all screen elements
void Draw_Tools();

//===================
//  INPUT
//===================

// handle command line arguments
int Parse_Args( int argc, char *argv[] );

// basic input capture
void Mouse_Input();

//====================================================================
//  MAIN
//====================================================================

int main( int argc, char *argv[] )
{
    Init_Textures();

    // parse command line args
    int mode = 0;

    if( ( mode = Parse_Args( argc, argv ) ) == 0 )
    {
        UTI_Fatal_Error( "Not enough/incorrect command line parameters" );
    }

    switch( mode )
    {
        case 1:
            if( Load_Textures( filename ) == 0 )
            {
                UTI_Fatal_Error( "Unable to load texture file" );
            }
            break;

        case 2:
            Generate_Texture();
            break;

        default:
            break;
    }


    // create display window
    if( GRA_Create_Display( "TexEdit", SCREEN_WIDTH, SCREEN_HEIGHT, RES_WIDTH, RES_HEIGHT ) == 0 )
    {
        UTI_Fatal_Error( "Unable to create screen" );
    }
    
    // load media
    if( GRA_Load_Font( "data/font" ) == 0 )
    {
        UTI_Fatal_Error( "Unable to load font data" );
    }
 
    if( GRA_Generate_Palette() == 0 )
    {
        UTI_Fatal_Error( "Unable to generate palette" );
    }

    if( Generate_Texture() == 0 )
    {
        UTI_Fatal_Error( "Unable to generate a new texure" );
    }

    Get_Current_Texture();

    // loop control
    int running = 1;
    while( running )
    {
        // clear the screen
        GRA_Clear_Screen();
       
        Mouse_Input();

        Draw_Tools();

        Draw_Current_Texture();

        // Refresh Window
        GRA_Refresh_Window();    

        running = GRA_Check_Quit();


        if( mouse_locked )
        {
            if( mouse_timer > 0 )
            {
                mouse_timer--;
            }
            else
            {
                mouse_locked = 0;
            }
        }

        GRA_Delay( 5 );
    }

    Save_Textures();

    Free_Textures();

    GRA_Close();

    return 0;
}

//====================================================================
//  FUNCTION BODIES
//====================================================================

//============================
//  TEXTURE FUNCTIONS
//============================

#include <string.h>

#define MAX_TEXTURES            1024

uint32_t                        *textures[MAX_TEXTURES];    // list of texture pointers
uint32_t                        texp = 0;                   // current texture
uint32_t                        texn = 0;                   // number of textures (current top of stack)


// saves current textures to a file
int Save_Textures()
{
    FILE *file;

    file = fopen( filename, "w" );
    if( file == NULL )
    {
        UTI_Print_Error( "Unable to create file" );
        return 0;
    }

    // create the file header
    char filetype[4] = "TXTR";
    fwrite( filetype, 4, 1, file );
    fwrite( &TEX_SIZE, sizeof( uint32_t ), 1, file );
    fwrite( &texn, sizeof( uint32_t ), 1, file );

    int i = 0;
    while( i < texn )
    {
        fwrite( textures[i], sizeof( uint32_t ) * TEX_SIZE * TEX_SIZE, 1, file );
        i++;
    }

    fclose( file );

    return 1;
}

// load textures from a file
int Load_Textures()
{
    FILE *file = NULL;
    char file_check[5];

    file = fopen( filename, "r" );
    if( file == NULL )
    {
        UTI_Print_Error( "Unable to open file" );
        return 0;
    }

    fread( file_check, 4, 1, file );
    file_check[4] = '\0';

    if( strcmp( file_check, "TXTR" ) != 0 )
    {
        UTI_Print_Error( "Cannot open file, invalid file type" );
        return 0;
    }

    fread( &TEX_SIZE, 4, 1, file );
    fread( &texn, 4, 1, file );

    printf( "File '%s' opened: %d textures, %dx%d\n", filename, texn, TEX_SIZE, TEX_SIZE );
    
    int i = 0;
    while( i < texn )
    {
        textures[i] = UTI_EC_Malloc( sizeof( uint32_t ) * TEX_SIZE * TEX_SIZE );
        fread( textures[i], sizeof( uint32_t ) * TEX_SIZE * TEX_SIZE, 1, file );
        i++;
    }

    current_texture = textures[0];

    PIXEL_SIZE = TXR_EDIT_W / TEX_SIZE;

    printf( "Textures read\n" );

    fclose( file );

    return 1;
}

void Init_Textures()
{
    int i = 0;
    while( i < MAX_TEXTURES )
    {
        textures[i++] = NULL;
    }
    
    return;
}

// create a new texture
int Generate_Texture()
{
    if( texn >= MAX_TEXTURES-1 )
    {
        UTI_Print_Error( "Cannot add texture, max number reached (1024)" );
        return 0;
    }

    textures[texn] = UTI_EC_Malloc( sizeof( uint32_t ) * TEX_SIZE * TEX_SIZE );

    // make texture blank
    int i;
    for( i = 0; i < (TEX_SIZE * TEX_SIZE); i++ )
    {
        textures[texn][i] = 0;          // 0 is black on the palette
    }

    texn++;
    return 1;
}

// for initialization
void Get_Current_Texture()
{
    current_texture = textures[texp];
    return;
}

// get next texture for editing
int Get_Next_Texture()
{
    if( texp >= MAX_TEXTURES-1 )
    {
        UTI_Print_Error( "Cannot select texture, max number reached" );
        return 0;
    }

    if( textures[++texp] == NULL )
    {
        // if texture doesnt exist, make it
        Generate_Texture();
    }


    current_texture = textures[texp];
    
    printf( "Current Texture = %d\n", texp );
    return 1;
}

// get previous texture
int Get_Prev_Texture()
{
    if( texp <= 0 )
    {
        UTI_Print_Error( "Cannot select texture, currently showing first texture" );
        return 0;
    }

    current_texture = textures[--texp];

    printf( "Current Texture = %d\n", texp );
    return 1;
}

// draw current texture to the editing window
void Draw_Current_Texture()
{
    if( current_texture == NULL )
    {
        return;
    }

    int i, j;
    for( i = 0; i < TEX_SIZE; i++ )
    {
        for( j = 0; j < TEX_SIZE; j++ )
        {
            GRA_Draw_Filled_Rectangle( (TXR_EDIT_X+i*PIXEL_SIZE), (TXR_EDIT_Y+j*PIXEL_SIZE), PIXEL_SIZE, PIXEL_SIZE, GRA_Get_Palette_Color(current_texture[j*TEX_SIZE+i]) );
        }
    }
    return;
}

// frees memory taken by textures
void Free_Textures()
{
    int i = 0;
    while( i < texn )
    {
        UTI_EC_Free( textures[i] );
        i++;
    }

    return;
}

//============================
//  GUI
//============================

// draws the GUI to the screen
void Draw_Tools()
{

    uint32_t WHITE = GRA_Create_Color( 255, 255, 255, 255 );

    // Draw Edit Area and Palette Area
    GRA_Draw_Hollow_Rectangle( TXR_EDIT_X-1, TXR_EDIT_Y-1, TXR_EDIT_W+1, TXR_EDIT_H+2, WHITE );
    GRA_Draw_Hollow_Rectangle( PAL_AREA_X-1, PAL_AREA_Y-1, PAL_AREA_W+1, PAL_AREA_H+2, WHITE );

    GRA_Simple_Text( "Texture", 32, 16, WHITE, 0, 0 );
    GRA_Simple_Text( "Palette", 356, 16, WHITE, 0, 0 );

    // Draw Color Selections
    GRA_Draw_Hollow_Rectangle( SELECTED_COLOR_X-1, SELECTED_COLOR_Y-1,  SELECTED_COLOR_W+1, SELECTED_COLOR_H+2, WHITE );
    GRA_Draw_Hollow_Rectangle( ERASE_COLOR_X-1, ERASE_COLOR_Y-1,  ERASE_COLOR_W+1, ERASE_COLOR_H+2, WHITE );

    GRA_Draw_Filled_Rectangle( SELECTED_COLOR_X, SELECTED_COLOR_Y, SELECTED_COLOR_W, SELECTED_COLOR_H, GRA_Get_Palette_Color( selected_color ) );
    GRA_Draw_Filled_Rectangle( ERASE_COLOR_X, ERASE_COLOR_Y, ERASE_COLOR_W, ERASE_COLOR_H, GRA_Get_Palette_Color( erase_color ) );

    GRA_Simple_Text( "Selected Colour", 72, SELECTED_COLOR_Y + 8, WHITE, 0, 0 );
    GRA_Simple_Text( "Erase Colour",    72, ERASE_COLOR_Y + 8, WHITE, 0, 0 );
 
    // draw texture selection arrows
    GRA_Draw_Hollow_Rectangle( TXR_SELECT_LEFT_X, TXR_SELECT_LEFT_Y, TXR_SELECT_W, TXR_SELECT_H, WHITE );
    GRA_Draw_Hollow_Rectangle( TXR_SELECT_RIGHT_X, TXR_SELECT_RIGHT_Y, TXR_SELECT_W, TXR_SELECT_H, WHITE );

    GRA_Simple_Text( "<", TXR_SELECT_LEFT_X + 8, TXR_SELECT_LEFT_Y + 2, WHITE, 0, 0 );
    GRA_Simple_Text( ">", TXR_SELECT_RIGHT_X + 8, TXR_SELECT_RIGHT_Y + 2, WHITE, 0, 0 );

    // TODO tidy
    int i = 0, j;
    for( i = 0; i < 16; i++ )
    {
        for( j = 0; j < 16; j++ )
        {
            GRA_Draw_Filled_Rectangle( PAL_AREA_X+i*16, PAL_AREA_Y+j*16, 16, 16, GRA_Get_Palette_Color(i*16+j) );
        }
    }
    return;
}

//============================
//  CONTROL AND INPUT
//============================

#include <ctype.h>

static int ac = 0;
static char **av = NULL;

int itoa( char *str )
{
    int len = strlen( str );
    int i;
    int n = 0;
    for( i = 0; i < len; i++ )
    {
        if( !isdigit( str[i] ) )
        {
            return -1;
        }
        
        n = n * 10 + ( str[i] - '0' );
    }

    return n;
}

// handle command line arguments
int Parse_Args( int argc, char *argv[] )
{
    // get command line args
    ac = argc;
    av = argv;

    if( ac < 2 )
    {
        printf( "Usage: %s <command> <filename> <size>\n", av[0] );
        printf( "Where  <command> = -o to open an existing file or -n to open a new file\n" );
        printf( "       <size>    = texture size in pixels, only needed when opening new files\n" );
        return 0;
    }

    if( ( strcmp( av[1], "-o" ) == 0 ) && ac > 2 )
    {
        filename = av[2];
        return 1;
    }

    if( ( strcmp( av[1], "-n" ) == 0 ) && ac > 3 )
    {
        filename = av[2];
        TEX_SIZE = itoa( av[3] );
        if( TEX_SIZE == -1 )
        {
            return 0;
        }
        PIXEL_SIZE = TXR_EDIT_W / TEX_SIZE;
        printf( "TEX_SIZE = %d, PIXEL_SIZE = %g\n", TEX_SIZE, PIXEL_SIZE );
        return 2;
    }

    return 0;
}

// reads mouse for user input, very simple implementation
void Mouse_Input()
{
    
    int m_button = 0, mousex, mousey, m_res_x, m_res_y;
    if( ( m_button = GRA_Get_Mouse_State( &mousex, &mousey ) ) != 0 )
    {
        m_res_x = floor( mousex / SCREEN_FACTOR_X );
        m_res_y = floor( mousey / SCREEN_FACTOR_Y );

        // check if mouse is in palette area
        if( ( m_res_x > PAL_AREA_X ) && ( m_res_x < PAL_AREA_X + PAL_AREA_W ) && 
            ( m_res_y > PAL_AREA_Y ) && ( m_res_y < PAL_AREA_Y + PAL_AREA_H ) )
        {
            int x_offset = ( m_res_x - PAL_AREA_X ) / 16;       // 16 is width of palette 'pixel'
            int y_offset = ( m_res_y - PAL_AREA_Y ) / 16;
            uint32_t color = ( x_offset * 16 + y_offset );

            if( m_button == 1 )
            {
                selected_color = color;
            }
            else if( m_button == 2 )
            {
                erase_color = color;
            }
        }

        // check if mouse is in texture edit area
        if( ( m_res_x > TXR_EDIT_X ) && ( m_res_x < TXR_EDIT_X + TXR_EDIT_W ) &&
            ( m_res_y > TXR_EDIT_Y ) && ( m_res_y < TXR_EDIT_Y + TXR_EDIT_H ) )
        {
            int x_offset = ( m_res_x - TXR_EDIT_X ) / PIXEL_SIZE;
            int y_offset = ( m_res_y - TXR_EDIT_Y ) / PIXEL_SIZE;
            
            if( m_button == 1 )
            {
                current_texture[y_offset * TEX_SIZE + x_offset] = selected_color;
            }
            else if( m_button == 2 )
            {
                current_texture[y_offset * TEX_SIZE + x_offset] = erase_color;
            }
        }

        // check if mouse is on buttons

        
        if( mouse_locked )
        {
            return;
        }

        if( ( m_res_x > TXR_SELECT_LEFT_X ) && ( m_res_x < TXR_SELECT_LEFT_X + TXR_SELECT_W ) &&
            ( m_res_y > TXR_SELECT_LEFT_Y ) && ( m_res_y < TXR_SELECT_LEFT_Y + TXR_SELECT_H ) )
        {
            Get_Prev_Texture();
        }

        if( ( m_res_x > TXR_SELECT_RIGHT_X ) && ( m_res_x < TXR_SELECT_RIGHT_X + TXR_SELECT_W ) &&
            ( m_res_y > TXR_SELECT_RIGHT_Y ) && ( m_res_y < TXR_SELECT_RIGHT_Y + TXR_SELECT_H ) )
        {
            Get_Next_Texture();
        }


        mouse_locked = 1;
        mouse_timer = 5;

    }


    return;
}
