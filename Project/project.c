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

int prime = 1234567;
int mytime = 0x5957;
char textstring[] = "text, more text, and even more text!";
int timeoutcount = 0;


/* Interrupt Service Routine */
void user_isr( void )
{
  if ( IFS(0) & 0x100 ) {
    IFSCLR(0) = 0x100;
    timeoutcount++;

    if ( timeoutcount == 10 ) {
      timeoutcount = 0;
      time2string( textstring, mytime );
      display_string( 3, textstring );
      display_update();
      tick( &mytime );

    }
  }

  if (IFS(0) & 0x80) {
    IFSCLR(0) = 0x80;
    int temp = PORTE;
    temp++;
    PORTE = temp;
  }

}

/* Lab-specific initialization goes here */
void labinit( void )
{

  TRISE = 0x0;

  // Set bit 8 to 1. (1 = input)
  TRISD |= 0x100;

  // Assignemnt 2b)
  // Timer init.
  T2CON = 0x70;           // 1:256 prescale value. (bits 6-4) 0111 0000 = 0x70
  TMR2 = 0;
  PR2 = 31250;            // == ((80000000 / 256) / 10)

  // Assignment 3
  IPC(2) = 0x1f;
  IFS(0) = 0;          // Clear the timer interrupt status flag
  IEC(0) = 0x180;      // Enable timer interrupts bit 7 for sw and 8 for timer

  // Surprise
  IPC(1) = 0x1f;       // Same prio as timer.


  T2CONSET = 0x8000;      // 0x8000 sets the 15th bit to 1. i.e starts the timer.

  enable_interrupt();
  return;
}


/* This function is called repetitively from the main program */
void labwork( void )
{
  //prime = nextprime( prime );
  //display_string( 0, itoaconv( prime ) );
  display_update();
}
