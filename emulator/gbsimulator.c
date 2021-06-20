/**
 * @file gbsimulator.c
 * @brief Simulation of the gameboy (executable)
 *
 * @author 
 * @date 2020
 */

#define _DEFAULT_SOURCE

#include <stdint.h>
#include <sys/time.h>
#include <error.h>

#include "sidlib.h"
#include "lcdc.h"
#include "gameboy.h"

// Key press bits
#define MY_KEY_UP_BIT    0x01
#define MY_KEY_DOWN_BIT  0x02
#define MY_KEY_RIGHT_BIT 0x04
#define MY_KEY_LEFT_BIT  0x08
#define MY_KEY_A_BIT     0x10
#define MY_KEY_B_BIT	 0x20
#define MY_KEY_START_BIT	 0x40
#define MY_KEY_SELECT_BIT	 0x80

#define SCALING_FACTOR 	3

// Global variables
gameboy_t gb;
struct timeval start;
struct timeval paused;

// ======================================================================
/**
 * @brief Converts a delay to gameboy cycles
 *
 * @param from cpu to plug into
 * @return converted gameboy cycles
 */
uint64_t get_time_in_GB_cyles_since(struct timeval* from)
{
    if (from != NULL) {

        // declare the current time and a time-interval to calculate the difference to the time provided as argument
        struct timeval time_now;
        struct timeval delta;
        // if initialization fails return
        if (gettimeofday(&time_now, NULL) != ERR_NONE) return 0;
        // if the time in argument is in the past return
        if (timercmp(from, &time_now, >=)) return 0;
        // calculate the time difference and convert it to gameboy cycle time
        timersub(&time_now, from, &delta);
        return (delta.tv_sec * GB_CYCLES_PER_S + (delta.tv_usec * GB_CYCLES_PER_S) / 1000000);
    }
    return 0;  
}

// ======================================================================
/**
 * @brief Executes the conversion between the pixels of the graphical interface and the provided data
 *
 * @param pixels pixels of the graphical interface
 * @param row number of rows of the screen
 * @param col number of cols of the screen
 * @param width width of the screen
 * @param grey the number to set the pixel to
 */
static void set_grey(guchar* pixels, int row, int col, int width, guchar grey)
{
    const size_t i = (size_t) (3 * (row * width + col)); // 3 = RGB
    pixels[i+2] = pixels[i+1] = pixels[i] = grey;
}

// ======================================================================
/**
 * @brief Generates the image
 *
 * @param pixels pixels of the graphical interface
 * @param height height of the screen
 * @param width width of the screen
 */
static void generate_image(guchar* pixels, int height, int width)
{
    // define a given amount of gameboy cycles
    uint64_t cycle = get_time_in_GB_cyles_since(&start);
    // run the gameboy until the predefined number of cycles
	int err = gameboy_run_until(&gb, cycle);
    if (err != ERR_NONE) fprintf(stderr, "gameboy_run_until() returns error: %i\n", err);
    
    // loop through the pixels, take the data and set the pixel accordingly
    for (int h = 0; h < height; h++) {
        for (int w = 0; w < width; w++) {
            uint8_t pixel_gameboy = 0;
            int err = image_get_pixel(&pixel_gameboy, &(gb.screen.display), w/SCALING_FACTOR, h/SCALING_FACTOR);
            if (err != ERR_NONE) fprintf(stderr, "image_get_pixel() returns error: %i\n", err);
            set_grey(pixels, h, w,  width, 255 - 85 * pixel_gameboy);
        } 
    }
}

// ======================================================================
#define do_key(X) \
    do { \
        if (! (psd->key_status & MY_KEY_ ## X ##_BIT)) { \
            psd->key_status |= MY_KEY_ ## X ##_BIT; \
            puts(#X " key pressed"); \
        } \
    } while(0)

static gboolean keypress_handler(guint keyval, gpointer data)
{
    simple_image_displayer_t* const psd = data;
    if (psd == NULL) return FALSE;

    switch(keyval) {
    case GDK_KEY_Up:
        do_key(UP);
        return TRUE;

    case GDK_KEY_Down:
        do_key(DOWN);
        return TRUE;

    case GDK_KEY_Right:
        do_key(RIGHT);
        return TRUE;

    case GDK_KEY_Left:
        do_key(LEFT);
        return TRUE;

    case 'A':
    case 'a':
        do_key(A);
        return TRUE;
        
    case 'S':
    case 's':
        do_key(B);
        return TRUE;
        
    case GDK_KEY_Page_Up:
        do_key(SELECT);
        return TRUE;

    case GDK_KEY_Page_Down:
        do_key(START);
        return TRUE;
        
	case GDK_KEY_space: 
		if(psd->timeout_id > 0) {
		    gettimeofday(&paused, NULL);
		} else {
			struct timeval time_now;
			gettimeofday(&time_now, NULL);
			timersub(&time_now, &paused, &paused);
			timeradd(&start, &paused, &start);
			timerclear(&paused);
		}
    }

    return ds_simple_key_handler(keyval, data);
}
#undef do_key

// ======================================================================
#define do_key(X) \
    do { \
        if (psd->key_status & MY_KEY_ ## X ##_BIT) { \
          psd->key_status &= (unsigned char) ~MY_KEY_ ## X ##_BIT; \
            puts(#X " key released"); \
        } \
    } while(0)

static gboolean keyrelease_handler(guint keyval, gpointer data)
{
    simple_image_displayer_t* const psd = data;
    if (psd == NULL) return FALSE;

    switch(keyval) {
    case GDK_KEY_Up:
        do_key(UP);
        return TRUE;

    case GDK_KEY_Down:
        do_key(DOWN);
        return TRUE;

    case GDK_KEY_Right:
        do_key(RIGHT);
        return TRUE;

    case GDK_KEY_Left:
        do_key(LEFT);
        return TRUE;

    case 'A':
    case 'a':
        do_key(A);
        return TRUE;
        
    case 'S':
    case 's':
        do_key(B);
        return TRUE;
        
    case GDK_KEY_Page_Up:
        do_key(SELECT);
        return TRUE;

    case GDK_KEY_Page_Down:
        do_key(START);
        return TRUE;
    }

    return FALSE;
}
#undef do_key

// ======================================================================
int main(int argc, char *argv[])
{
    if (argc < 2) {
        error(argv[0], 1, "Please provide input_file!");
        return 1;
    }

    const char* const filename = argv[1];
    // create the gameboy
    int err = gameboy_create(&gb, filename);
    // in case of error return with error code
    if (err != ERR_NONE) {
        gameboy_free(&gb);
        fprintf(stderr, "Error while creating gameboy: %i\n", err);
        return err;
    }    
    
    // initialize the global variable start
    int errtime = gettimeofday(&start, NULL);
    // in case of error return with error code
    if (errtime != ERR_NONE) {
        gameboy_free(&gb);
        fprintf(stderr, "Error while init time: %i\n", errtime);
        return errtime;
    }
    // initialize the global variable paused
    timerclear(&paused);
    
    // launch the program, generate image and run the gameboy cycle
    sd_launch(&argc, &argv,
                  sd_init("Gameboy", LCD_WIDTH * SCALING_FACTOR, LCD_HEIGHT * SCALING_FACTOR, 40,
                          generate_image, keypress_handler, keyrelease_handler));

    // free the gameboy at the end of execution
    gameboy_free(&gb);
    
    return err;
}
