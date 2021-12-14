/* mipslab.h
   Header file for all labs.
   This file written 2015 by F Lundevall
   Some parts are original code written by Axel Isaksson

   Latest update 2015-08-28 by F Lundevall

   For copyright and licensing, see file COPYING */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pic32mx.h>

/* Declare display-related functions from mipslabfunc.c */
void display_image(int x, uint8_t data[128][4]);
void display_init(void);
void display_string(int line, char *s);
void display_update(void);
uint8_t spi_send_recv(uint8_t data);

void init();
void user_isr(void);

int get_number(int min, int max);

/* Declare project-related functions */
void run(void);
void clear_display(void);
void clear_text_buffer(void);
void copy_board(const char matrix[128][32], char matrix_copy[128][32]);
int add_bob_to_board(int pos_x, int pos_y);
int pow2(int base, int exp);
void display_current_board();
void place_obstacle(int pos_x, int _num);
void set_next_obstacle(const int obstacle_label);
void move(int *flag_x, int *flag_y);
int is_wall(int pos_x);
void add_score(int x_coordinate, int y_coordinate);
void highscore(int list[3]);
int * get_name(void);
int * conv_to_char_sequence(int number, int size);
int display_menu(void);
/* Declare display_debug - a function to help debugging.

   After calling display_debug,
   the two middle lines of the display show
   an address and its current contents.

   There's one parameter: the address to read and display.

   Note: When you use this function, you should comment out any
   repeated calls to display_image; display_image overwrites
   about half of the digits shown by display_debug.
*/
void display_debug(volatile int * const addr);

/* Declare bitmap array containing font */
extern const uint8_t const font[128*8];

/* Declare text buffer for display output */
extern char textbuffer[4][16];

/* Declare OLED specific information */
extern const int oled_x;
extern const int oled_y;
extern const int num_of_pages;

char old_names[3][5];
/* Declare constants */
const char const bob[9][9];
const char const board[128][32];
char board_copy[128][32];
uint8_t board_to_display[128][4];

const int BOARD_WIDTH;
const int BOARD_HEIGHT;
const int BOB_WIDTH;
const int BOB_HEIGHT;
const int BOB_MAX_Y;
const int BOB_MIN_Y;
const int BOB_MAX_X;
const int BOB_MIN_X;

TIME_BETWEEN_OBSTACLES;

/* Declare current status variables */
int jump_flag;  // 0 = on the ground, 1 = jump, 2 = fall
int dash_flag;  // "dash" in x-direction
int crash_flag;
int time_out;
int bob_x;
int bob_y;
int score;

const int LENGTH_OF_OBS_ARR;

int obstacles[12];
int obstacle_pos[12];

const int minimal_distance;
const int NOTHING;
const int ROCK;
const int WALL;
const int ORB;

const char const rock[9][7];
const char const orb[2][28];
const char const wall[2][28];
