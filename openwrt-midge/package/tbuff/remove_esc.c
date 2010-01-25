#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
					
#include "demon.h"

#define ESC 27


static void state1(int c, struct port *p)
{
  short x, y, f;

  switch(c) {
    case '[': /* ESC [ */
      p->esc_s = 2;
      return;
    case '(': /* ESC ( */
      p->esc_s = 4;
      return;
    case ')': /* ESC ) */
      p->esc_s = 5;
      return;
    case '#': /* ESC # */
      p->esc_s = 6;
      return;
    case 'P': /* ESC P (DCS, Device Control String) */
      p->esc_s = 7;
      return;
    default:
      /* ALL IGNORED */
      break;
  }
  p->esc_s = 0;
}



/*
 * ESC [ ... was seen the last time. Process next character.
 */
static int state2(int c, struct port *p)
{

  if (c == 'J')
  {
  	p->esc_s = 0;
  	return 2;
  }
  /* See if a number follows */
  if (c >= '0' && c <= '9') {
    return;
  }
  /* Separation between numbers ? */
  if (c == ';') {
    return;
  }
  /* ESC [ ? sequence */
  if (c == '?')
  {
    p->esc_s = 3;
    return;
  }

  p->esc_s = 0;
  return;
}


/*
 * ESC [ ? ... seen.
 */
static void state3(int c, struct port *p)
{
  /* See if a number follows */
  if (c >= '0' && c <= '9') {
    return;
  }
  p->esc_s = 0;
  return;
}

/*
 * ESC ( Seen.
 */
static void state4(int c, struct port *p)
{
  p->esc_s = 0;
}

/*
 * ESC ) Seen.
 */
static void state5(int c, struct port *p)
{
  p->esc_s = 0;
}

/*
 * ESC # Seen.
 */
static void state6(int c, struct port *p)
{
  p->esc_s = 0;
}

/*
 * ESC P Seen.
 */
static void state7(int c, struct port *p)
{
  /*
   * Device dependant control strings. The Minix virtual console package
   * uses these sequences. We can only turn cursor on or off, because
   * that's the only one supported in termcap. The rest is ignored.
   */
  static int state = 0;

  if (c == ESC) {
    state = 1;
    return;
  }
  if (state == 1) {
    state = 0;
    p->esc_s = 0;
    return;
  }
  return;
}



int out(int ch, struct port *p)
{
  int f;
  unsigned char c;
  int go_on = 0;
  
//  printf("ch = %x p->esc_s = %i\n", ch, p->esc_s);
  if (!ch)
    return 1;

  c = (unsigned char)ch;

  switch (c) {
    case 24:
    case 26:  /* Cancel escape sequence. */
      p->esc_s = 0;
      break;
    case ESC: /* Begin escape sequence */
      p->esc_s = 1;
      break;
    case 128+ESC: /* Begin ESC [ sequence. */
      p->esc_s = 2;
      break;
    default:
      go_on = 1;
      break;
  }
  if (!go_on)
    return 0;

  /* Now see which state we are in. */
  switch (p->esc_s) {
    case 0:
      	return 1;
      break;
    case 1: /* ESC seen */
      state1(c, p);
      break;
    case 2: /* ESC [ ... seen */
      if (state2(c, p) == 2) return 2;
      break;
    case 3:
      state3(c, p);
      break;
    case 4:
      state4(c, p);
      break;
    case 5:
      state5(c, p);
      break;
    case 6:
      state6(c, p);
      break;
    case 7:
      state7(c, p);
      break;
  }
  return 0;
}
