/* mipslabwork.c

   This file written 2015 by F Lundevall
   Updated 2017-04-21 by F Lundevall

   This file should be changed by YOU! So you must
   add comment(s) here with your name(s) and date(s):

   This file modified 2017-04-31 by Ture Teknolog 

   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "mipslab.h"  /* Declatations for these labs */

int mytime = 0x5957;
char textstring[] = "text, more text, and even more text!";
int timeoutcount = 0;


/* Interrupt Service Routine */
void user_isr( void )
{
  return;
}

/* Lab-specific initialization goes here */
void labinit( void )
{
  volatile int* tris_e = (volatile int*) 0xbf886100;

  // Assignment 1c)
  // Declares Port E as output
  int temp1 = *tris_e;
  temp1 >> 8;          // Shifts to the right. 
  temp1 << 8;          // Shifts back left, filling out with 0 at bits 0-7.
  *tris_e = temp1;

  // Assignment 1e)
  // Set bits 11 - 5 to 1. (1 = input)
  TRISD |= 0xfe0;
  
  // Assignemnt 2b)
  // Timer init.
  T2CON = 0x70;           // 1:256 prescale value. (bits 6-4) 0111 0000 = 0x70
  PR2 = 31250;            // == ((80000000 / 256) / 10)
  T2CONSET = 0x8000;      // 0x8000 sets the 15th bit to 1. i.e starts the timer.
  
  return;
}


/* This function is called repetitively from the main program */
void labwork( void )
{
  volatile int* port_e = (volatile int*) 0xbf886110;
 
  int btn = getbtns();
  int sw = getsw();
  // 3 5 7
  if(btn == 0x1 || btn == 0x3 || btn == 0x5 || btn == 0x7) {  // btn2
    mytime = (mytime & 0xff0f) | (sw << 4);
  }
  // 3 6 7
  if(btn == 0x2 || btn == 0x3 || btn == 0x6 || btn == 0x7) {  // btn3
    mytime = (mytime & 0xf0ff) | (sw << 8);
  }  
  // 5 6 7
  if(btn == 0x4 || btn == 0x5 || btn  == 0x6 || btn == 0x7) {  // btn4
    mytime = (mytime & 0x0fff) | (sw << 12);
  } 


  // Assignment 2c)
  if(IFS(0) & 0x100) {
    IFSCLR(0) = 0x100;
    timeoutcount++;
   

    if(timeoutcount == 10) {
      timeoutcount = 0;
      time2string( textstring, mytime );
      display_string( 3, textstring );
      display_update();
      tick( &mytime );
      display_image(96, icon);
      
      // Assignment 1d) increment tick counter (green LEDs)
      int temp = *port_e;
      temp++;
      *port_e = temp;
    
    }
  }

}







