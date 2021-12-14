/* mipslabfunc.c
 This file written 2015 by F Lundevall
 Some parts are original code written by Axel Isaksson
 For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "mipslab.h"  /* Declatations for these labs */

/* Declare a helper function which is local to this file */
static void num32asc( char * s, int );

#define DISPLAY_CHANGE_TO_COMMAND_MODE (PORTFCLR = 0x10)
#define DISPLAY_CHANGE_TO_DATA_MODE (PORTFSET = 0x10)

#define DISPLAY_ACTIVATE_RESET (PORTGCLR = 0x200)
#define DISPLAY_DO_NOT_RESET (PORTGSET = 0x200)

#define DISPLAY_ACTIVATE_VDD (PORTFCLR = 0x40)
#define DISPLAY_ACTIVATE_VBAT (PORTFCLR = 0x20)

#define DISPLAY_TURN_OFF_VDD (PORTFSET = 0x40)
#define DISPLAY_TURN_OFF_VBAT (PORTFSET = 0x20)

int TIME_BETWEEN_SCORES = 10;
int difficulty_level = 10;
int dis = 10;
int difficulty_counter;
int time_obstacles;
/* quicksleep:
 A simple function to create a small delay.
 Very inefficient use of computing resources,
 but very handy in some special cases. */
void quicksleep(int cyc) {
int i;
for(i = cyc; i > 0; i--);
}

void init() {

	/* Set BTN4, BTN3 and BTN2 as input */
	TRISD |= 0xFE0;

	/* Timer init */
	T2CON = 0x70;	// Prescale value 256
	TMR2 = 0;
	PR2 = 31250; // 80MHz / 256 / 10

	/* Interrupt init */
	IPC(2) = 0x1F; // Priority
	IFS(0) = 0;
	IEC(0) = 0x100;
	time_out = 0;
	time_obstacles = 0;
	difficulty_counter = 0;

	enable_interrupt();
}



/* display_debug
 A function to help debugging.
 After calling display_debug,
 the two middle lines of the display show
 an address and its current contents.
 There's one parameter: the address to read and display.
 Note: When you use this function, you should comment out any
 repeated calls to display_image; display_image overwrites
 about half of the digits shown by display_debug.
*/
void display_debug( volatile int * const addr )
{
display_string( 1, "Addr" );
display_string( 2, "Data" );
num32asc( &textbuffer[1][6], (int) addr );
num32asc( &textbuffer[2][6], *addr );
display_update();
}

uint8_t spi_send_recv(uint8_t data) {
while(!(SPI2STAT & 0x08));
SPI2BUF = data;
while(!(SPI2STAT & 1));
return SPI2BUF;
}

int display_menu(void) {
	while(1) {
		display_string( 0, "MENU" );
		display_string( 1, "BTN4->HIGHSCORE" );
		display_string( 2, "BTN3->GAME     " );
		display_update();

		// BTN4 -> VIEW HIGHSCORE
		if(PORTD &= 0x80) {
			return 0;
		}
		// BTN3 -> RUN GAME
		if(PORTD &= 0x40) {
			return 1;
		}
	}

}

int get_number(int min, int max) {
	 int current_time = (int) TMR2 & 0xFFFF;
	 return current_time % max + min;
}

void set_next_obstacle(const int obstacle_label) {
	int i;
	for(i = 0; i < LENGTH_OF_OBS_ARR; i++) {
		if(obstacle_pos[i] == -10 && obstacles[i] == NOTHING) {
			obstacles[i] = ROCK;
			if (i%2 == 1) {
				obstacles[i] = WALL;
			}
			if (i%3 == 2) {
				obstacles[i] = ORB;
			}
			obstacle_pos[i] = 115 + dis;
			return;
		}
	}
}

int * conv_to_char_sequence(int number, int size) {
	int array[size];
	int i,c,number_copy;
	int temp;
	number_copy = number;

	for(i = size-1; i >= 0; i--){
		temp = number_copy % 10;
		c = (int) (temp + '0');
		number_copy = (number_copy - (int) temp) / 10;
		array[i] = c;
	}
	return array;
}

void add_to_board(int x_coordinate, int y_coordinate, int *chars, int size) {
	int i,k,c;
	for(i = 0; i < size; i++){
		c = *(chars+i);
		for(k = 0; k < 8; k++) {
			board_to_display[(x_coordinate)+k+(i*8)][y_coordinate] = font[c*8 + k];
		}
	}
}

void add_score(int x_coordinate, int y_coordinate) {
	//int chars;
	int i,k,c,temp;
	int score_copy = score;

	int *chars;
	chars = conv_to_char_sequence(score, 4);
	for(i = 0; i < 4; i++){
		c = *(chars+i);
		for(k = 0; k < 8; k++) {
			board_to_display[(x_coordinate)+k+(i*8)][y_coordinate] = font[c*8 + k];
		}
	}
}

int * get_name( void ) {
	int hej[5] = {65,32,65,32,65};

	copy_board(board, board_copy);
	display_current_board();

	int counter = 0;
	int letter = 65;
	while (1) {
		add_to_board(50, 1, hej, 5);
		display_image(0, board_to_display);
		if(PORTD &= 0x80) {
			letter--;
			quicksleep(1000000);
		}
		if(PORTD &= 0x40) {
			letter++;
			quicksleep(1000000);
		}
		if(PORTD &= 0x20) {
			hej[counter] = letter;
			counter = counter + 2;
			quicksleep(1000000);
			if (counter == 6) {
				return hej;
			}
			letter = 65;

		}
		hej[counter] = letter;
	}
	return hej;
}

void update_highscore(int* old_highscore, int score) {
	int *first_place = (old_highscore+0);
	int *second_place = (old_highscore+1);
	int *third_place = (old_highscore+2);

	int* ascii_codes = get_name(); //(0,2,4)

	if(score < 0) {
		return;
	}
	else if(score <= *second_place) {
		*third_place = score;
		old_names[2][0] = *ascii_codes;
		old_names[2][2] = *(ascii_codes+2);
		old_names[2][4] = *(ascii_codes+4);
		return;
	}
	else if(score <= *first_place) {
		*third_place = *second_place;
		old_names[2][0] = old_names[1][0];
		old_names[2][2] = old_names[1][2];
		old_names[2][4] = old_names[1][4];

		*second_place = score;
		old_names[1][0] = *ascii_codes;
		old_names[1][2] = *(ascii_codes+2);
		old_names[1][4] = *(ascii_codes+4);
		return;
	}
	else if(score > *first_place) {
		*third_place = *second_place;
		old_names[2][0] = old_names[1][0];
		old_names[2][2] = old_names[1][2];
		old_names[2][4] = old_names[1][4];

		*second_place = *first_place;
		old_names[1][0] = old_names[0][0];
		old_names[1][2] = old_names[0][2];
		old_names[1][4] = old_names[0][4];

		*first_place = score;
		old_names[0][0] = *ascii_codes;
		old_names[0][2] = *(ascii_codes+2);
		old_names[0][4] = *(ascii_codes+4);
		return;
	}

	return;
}



char * create_highscore_entry(int rank, int score, int size) {
	char array[size];
	int i,c,score_copy;
	int temp;
	score_copy = score;

	array[0]=rank+'0';
	array[1]=' ';
	array[2]= old_names[rank-1][0];//'A';
	array[3]= old_names[rank-1][2];//'B';
	array[4]= old_names[rank-1][4];//'C';
	array[5]=' ';

	for(i = 9; i > 5; i--){
		temp = score_copy % 10;
		c = temp + '0';
		score_copy = (score_copy - (int) temp) / 10;
		array[i] = c;
	}
	return array;
}


void display_highscore(int* highscore) {
	display_string(0, "HIGHSCORE");
	int i;
	for (i=0;i<3;i++){
		int score = *(highscore+i);
		char *highscore_entry = create_highscore_entry(i+1, score, 10);
		display_string(i+1, highscore_entry);
	}
	display_update();
}

void user_isr(void){
	IFSCLR(0) = 0x100;
	time_out++;
	time_obstacles++;

	if (time_out == TIME_BETWEEN_SCORES) {
		time_out = 0;
		score++;
	}
	if (time_obstacles == TIME_BETWEEN_OBSTACLES){
		time_obstacles = 0;
		set_next_obstacle(ROCK);
		difficulty_counter++;
		if ( difficulty_counter == 10) {
			difficulty_counter = 0;
			if (TIME_BETWEEN_OBSTACLES != 1){
				TIME_BETWEEN_OBSTACLES--;
			}

		}
	}

}

// Borrowed function
int pow2(int base, int exp){
	if(exp < 0)
	  return -1;
	  int result = 1;
	  while (exp){
      if (exp & 1)
          result *= base;
      exp >>= 1;
      base *= base;
	  }
	  return result;
}

// Copy board. Dimensions for both matricies must be 128x32.
void copy_board(const char matrix[128][32], char matrix_copy[128][32]) {
	int x,y;
	for (x = 0; x < BOARD_WIDTH; x++){
		for (y = 0; y < BOARD_HEIGHT; y++)
			matrix_copy[x][y] = matrix[x][y];
	}
}


int is_wall(int pos_x) {
	int i, j;
	for (j = -1; j < BOB_WIDTH; j++) {
		for (i = 0; i < LENGTH_OF_OBS_ARR; i++) {
			if ((obstacle_pos[i] == (pos_x)+j || obstacle_pos[i]+1 == (pos_x+j)) && obstacles[i] == WALL) {
				return 1;
			}
		}
	}
	return 0;
}


// Add bob into board into coordinate (pos_x, pos_y)
int add_bob_to_board(int pos_x, int pos_y) {
	int x,y;
	int game_end_flag = 0;
	for (x = 0; x < BOB_HEIGHT; x++){
		for (y = 0; y < BOB_WIDTH; y++){
			if (is_wall(pos_x) && dash_flag != 0) {
				game_end_flag = 0;
			}
			else if (bob[x][y] == '1' && board_copy[x+pos_x][y+pos_y] == '1') {
				game_end_flag = 1;
			}
			board_copy[x+pos_x][y+pos_y] = bob[x][y];
		}
	}
	return game_end_flag;
}


void place_obstacle(int pos_xx, int obs_num){
	int pos_y, len_x, len_y;
	if (obs_num == ROCK) {
		pos_y = 32-7-1;
		len_x = sizeof(rock)/sizeof(rock[0]);
		len_y = sizeof(rock[0])/sizeof(rock[0][0]);
		}
	if (obs_num == WALL) {
		pos_y = 32-28-1;
		len_x = sizeof(wall)/sizeof(wall[0]); //2
		len_y = sizeof(wall[0])/sizeof(wall[0][0]); // 28
		}
	if (obs_num == ORB) {
		pos_y = 32-28-1;
		len_x = sizeof(orb)/sizeof(orb[0]); //2
		len_y = sizeof(orb[0])/sizeof(orb[0][0]); // 28
		}
		int x,y;
		for (x = 0; x < len_x; x++){
			for (y = 0; y < len_y; y++) {
				if (obs_num == ROCK) {
					board_copy[x+pos_xx][y+pos_y] = rock[x][y];
				}
				if (obs_num == WALL) {
					board_copy[x+pos_xx][y+pos_y] = wall[x][y];
				}
				if (obs_num == ORB) {
					board_copy[x+pos_xx][y+pos_y] = orb[x][y];
				}
			}

		}

}

void display_current_board() {
	int x,y,res,p,c,i;
	for (y = 0, p = 0; y < BOARD_HEIGHT; y += 8, p++){	//32
			for (x = 0; x < BOARD_WIDTH; x++){	//128

				res = 0;
				for (i = 0; i < 8; i++){
					c = (int) board_copy[x][y+i];
					if (c == 48){
						c = 0;
					} else {
						c = 1;
					}
					res += c*(pow2(2,(i)));

				}
				board_to_display[x][p] = (uint8_t) res;
		}
	}
}

void clear_display(void) {
	uint8_t data = 0;
	int page, i;
	for(page = 0; page < num_of_pages; page++){
		DISPLAY_CHANGE_TO_COMMAND_MODE;
		spi_send_recv(0x22);	//Set page command
		spi_send_recv(page);	//page number (row)
		spi_send_recv(0x0);		//set low nibble of column
		spi_send_recv(0x10);	//set high nibble of column
		DISPLAY_CHANGE_TO_DATA_MODE;
		for(i = 0; i < oled_x; i++) {
			spi_send_recv(data);
		}
	}
}

void move(int *flag_x, int *flag_y) {
	if (*flag_x == 1) {
		// Update x-coordinate for bob
		if(bob_x >= BOB_MAX_X) {
			*flag_x = 2;
		}
		bob_x = bob_x + 2;
	}
	if (*flag_x == 2) {
		bob_x--;
		if(bob_x == BOB_MIN_X) {
				*flag_x = 0;
		}
	}
	// Assert that bob does not exit screen in x-direction
	if (bob_x >= 115){
		bob_x = 114;
	}

	if(*flag_y == 1){
		// Update y-coordinate for bob
		bob_y--;
		if(bob_y <= BOB_MAX_Y) {
			*flag_y = 2;
		}
	}
	if(*flag_y == 2) {
		bob_y++;
		if(bob_y == BOB_MIN_Y) {
				*flag_y = 0;
		}
	}
}

void clear_text_buffer(void) {
	int i;
	for(i = 0; i < 4; i++){
		display_string(i, "                ");
	}
}

void display_init(void) {
	DISPLAY_CHANGE_TO_COMMAND_MODE;
	quicksleep(10);
	DISPLAY_ACTIVATE_VDD;
	quicksleep(1000000);

	spi_send_recv(0xAE);
	DISPLAY_ACTIVATE_RESET;
	quicksleep(10);
	DISPLAY_DO_NOT_RESET;
	quicksleep(10);

	spi_send_recv(0x8D);
	spi_send_recv(0x14);

	spi_send_recv(0xD9);
	spi_send_recv(0xF1);

	DISPLAY_ACTIVATE_VBAT;
	quicksleep(10000000);

	spi_send_recv(0xA1);
	spi_send_recv(0xC8);

	spi_send_recv(0xDA);
	spi_send_recv(0x20);

	spi_send_recv(0xAF);

	clear_text_buffer();
	clear_display();
}


void display_string(int line, char *s) {
int i;
if(line < 0 || line >= 4)
	return;
if(!s)
	return;

for(i = 0; i < 16; i++)
	if(*s) {
		textbuffer[line][i] = *s;
		s++;
	} else
		textbuffer[line][i] = ' ';
}

void display_image(int x, uint8_t data[128][4]) {
	int i, j;

	for(i = 0; i < 4; i++) {
		DISPLAY_CHANGE_TO_COMMAND_MODE;

		spi_send_recv(0x22);
		spi_send_recv(i);

		spi_send_recv(x & 0xF);
		spi_send_recv(0x10 | ((x >> 4) & 0xF));

		DISPLAY_CHANGE_TO_DATA_MODE;
		for(j = 0; j < 128; j++){
				spi_send_recv(data[j][i]);
		}
	}
}

void display_update(void) {
int i, j, k;
int c;
for(i = 0; i < 4; i++) {
	DISPLAY_CHANGE_TO_COMMAND_MODE;
	spi_send_recv(0x22);
	spi_send_recv(i);

	spi_send_recv(0x0);
	spi_send_recv(0x10);

	DISPLAY_CHANGE_TO_DATA_MODE;

	for(j = 0; j < 16; j++) {
		c = textbuffer[i][j];
		if(c & 0x80)
			continue;

		for(k = 0; k < 8; k++)
			spi_send_recv(font[c*8 + k]);
	}
}
}

/* Helper function, local to this file.
 Converts a number to hexadecimal ASCII digits. */
static void num32asc( char * s, int n )
{
int i;
for( i = 28; i >= 0; i -= 4 )
  *s++ = "0123456789ABCDEF"[ (n >> i) & 15 ];
}

/*
* nextprime
*
* Return the first prime number larger than the integer
* given as a parameter. The integer must be positive.
*/
#define PRIME_FALSE   0     /* Constant to help readability. */
#define PRIME_TRUE    1     /* Constant to help readability. */
int nextprime( int inval )
{
 register int perhapsprime = 0; /* Holds a tentative prime while we check it. */
 register int testfactor; /* Holds various factors for which we test perhapsprime. */
 register int found;      /* Flag, false until we find a prime. */

 if (inval < 3 )          /* Initial sanity check of parameter. */
 {
   if(inval <= 0) return(1);  /* Return 1 for zero or negative input. */
   if(inval == 1) return(2);  /* Easy special case. */
   if(inval == 2) return(3);  /* Easy special case. */
 }
 else
 {
   /* Testing an even number for primeness is pointless, since
    * all even numbers are divisible by 2. Therefore, we make sure
    * that perhapsprime is larger than the parameter, and odd. */
   perhapsprime = ( inval + 1 ) | 1 ;
 }
 /* While prime not found, loop. */
 for( found = PRIME_FALSE; found != PRIME_TRUE; perhapsprime += 2 )
 {
   /* Check factors from 3 up to perhapsprime/2. */
   for( testfactor = 3; testfactor <= (perhapsprime >> 1) + 1; testfactor += 1 )
   {
     found = PRIME_TRUE;      /* Assume we will find a prime. */
     if( (perhapsprime % testfactor) == 0 ) /* If testfactor divides perhapsprime... */
     {
       found = PRIME_FALSE;   /* ...then, perhapsprime was non-prime. */
       goto check_next_prime; /* Break the inner loop, go test a new perhapsprime. */
     }
   }
   check_next_prime:;         /* This label is used to break the inner loop. */
   if( found == PRIME_TRUE )  /* If the loop ended normally, we found a prime. */
   {
     return( perhapsprime );  /* Return the prime we found. */
   }
 }
 return( perhapsprime );      /* When the loop ends, perhapsprime is a real prime. */
}

/*
* itoa
*
* Simple conversion routine
* Converts binary to decimal numbers
* Returns pointer to (static) char array
*
* The integer argument is converted to a string
* of digits representing the integer in decimal format.
* The integer is considered signed, and a minus-sign
* precedes the string of digits if the number is
* negative.
*
* This routine will return a varying number of digits, from
* one digit (for integers in the range 0 through 9) and up to
* 10 digits and a leading minus-sign (for the largest negative
* 32-bit integers).
*
* If the integer has the special value
* 100000...0 (that's 31 zeros), the number cannot be
* negated. We check for this, and treat this as a special case.
* If the integer has any other value, the sign is saved separately.
*
* If the integer is negative, it is then converted to
* its positive counterpart. We then use the positive
* absolute value for conversion.
*
* Conversion produces the least-significant digits first,
* which is the reverse of the order in which we wish to
* print the digits. We therefore store all digits in a buffer,
* in ASCII form.
*
* To avoid a separate step for reversing the contents of the buffer,
* the buffer is initialized with an end-of-string marker at the
* very end of the buffer. The digits produced by conversion are then
* stored right-to-left in the buffer: starting with the position
* immediately before the end-of-string marker and proceeding towards
* the beginning of the buffer.
*
* For this to work, the buffer size must of course be big enough
* to hold the decimal representation of the largest possible integer,
* and the minus sign, and the trailing end-of-string marker.
* The value 24 for ITOA_BUFSIZ was selected to allow conversion of
* 64-bit quantities; however, the size of an int on your current compiler
* may not allow this straight away.
*/
#define ITOA_BUFSIZ ( 24 )
char * itoaconv( int num )
{
register int i, sign;
static char itoa_buffer[ ITOA_BUFSIZ ];
static const char maxneg[] = "-2147483648";

itoa_buffer[ ITOA_BUFSIZ - 1 ] = 0;   /* Insert the end-of-string marker. */
sign = num;                           /* Save sign. */
if( num < 0 && num - 1 > 0 )          /* Check for most negative integer */
{
  for( i = 0; i < sizeof( maxneg ); i += 1 )
  itoa_buffer[ i + 1 ] = maxneg[ i ];
  i = 0;
}
else
{
  if( num < 0 ) num = -num;           /* Make number positive. */
  i = ITOA_BUFSIZ - 2;                /* Location for first ASCII digit. */
  do {
    itoa_buffer[ i ] = num % 10 + '0';/* Insert next digit. */
    num = num / 10;                   /* Remove digit from number. */
    i -= 1;                           /* Move index to next empty position. */
  } while( num > 0 );
  if( sign < 0 )
  {
    itoa_buffer[ i ] = '-';
    i -= 1;
  }
}
/* Since the loop always sets the index i to the next empty position,
 * we must add 1 in order to return a pointer to the first occupied position. */
return( &itoa_buffer[ i + 1 ] );
}
