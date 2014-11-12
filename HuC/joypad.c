/*
   The MIT License (MIT)

   Copyright (c) 2014 Antonio SJ Musumeci <trapexit@spawn.link>

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

#define BLINKY0       0x01CA
#define BLINKY1       0x81CA
#define CAVE          0x8154
#define LEFT_RIGHT    0x0050
#define LUNARLANDER   0x0054
#define PONG_1P       0x0012
#define PONG_2P       0x3012
#define ROCKET        0x0850
#define SPACEINVADERS 0x0070
#define SYZYGY0       0xC1C8
#define SYZYGY1       0xC9C8
#define SYZYGY_SELECT 0xC000
#define TANK          0x0174
#define TETRIS        0x00F0

#define JOY_UP_SEL    0x14
#define JOY_UP_RUN    0x18
#define JOY_UP_II     0x12
#define JOY_UP_I      0x11
#define JOY_RIGHT_SEL 0x24
#define JOY_RIGHT_RUN 0x28
#define JOY_RIGHT_II  0x22
#define JOY_RIGHT_I   0x21
#define JOY_DOWN_SEL  0x44
#define JOY_DOWN_RUN  0x48
#define JOY_DOWN_II   0x42
#define JOY_DOWN_I    0x41
#define JOY_LEFT_SEL  0x84
#define JOY_LEFT_RUN  0x88
#define JOY_LEFT_II   0x82
#define JOY_LEFT_I    0x81

extern unsigned int keymask;

char
key_pressed(const unsigned char key)
{
  switch(keymask)
    {
    case BLINKY0:
    case BLINKY1:
      return blinky_keys(key);
    case CAVE:
      return cave_keys(key);
    case LEFT_RIGHT:
      return left_right_keys(key);
    case LUNARLANDER:
      return lunarlander_keys(key);
    case PONG_1P:
    case PONG_2P:
      return pong_keys(key);
    case ROCKET:
      return rocket_keys(key);
    case SPACEINVADERS:
      return spaceinvaders_keys(key);
    case SYZYGY0:
    case SYZYGY1:
    case SYZYGY_SELECT:
      return syzygy_keys(key);
    case TANK:
      return tank_keys(key);
    case TETRIS:
      return tetris_keys(key);

    case 0x0001:
    case 0x0002:
    case 0x0004:
    case 0x0008:
    case 0x0010:
    case 0x0020:
    case 0x0040:
    case 0x0080:
    case 0x0100:
    case 0x0200:
    case 0x0400:
    case 0x0800:
    case 0x1000:
    case 0x2000:
    case 0x4000:
    case 0x8000:
      return (joy(0) & JOY_I);

    default:
      return all_keys(key);
    }

  return 0;
}

static
char
all_keys(char key)
{
  switch(key)
    {
    case 0x01:
      return (joy(0) & (JOY_UP | JOY_SEL));
    case 0x02:
      return (joy(0) & (JOY_UP | JOY_RUN));
    case 0x03:
      return (joy(0) & (JOY_UP | JOY_II));
    case 0x0C:
      return (joy(0) & (JOY_UP | JOY_I));

    case 0x04:
      return (joy(0) & (JOY_LEFT | JOY_SEL));
    case 0x05:
      return (joy(0) & (JOY_LEFT | JOY_RUN));
    case 0x06:
      return (joy(0) & (JOY_LEFT | JOY_II));
    case 0x0D:
      return (joy(0) & (JOY_LEFT | JOY_I));

    case 0x07:
      return (joy(0) & (JOY_RIGHT | JOY_SEL));
    case 0x08:
      return (joy(0) & (JOY_RIGHT | JOY_RUN));
    case 0x09:
      return (joy(0) & (JOY_RIGHT | JOY_II));
    case 0x0E:
      return (joy(0) & (JOY_RIGHT | JOY_I));

    case 0x0A:
      return (joy(0) & (JOY_DOWN | JOY_SEL));
    case 0x00:
      return (joy(0) & (JOY_DOWN | JOY_RUN));
    case 0x0B:
      return (joy(0) & (JOY_DOWN | JOY_II));
    case 0x0F:
      return (joy(0) & (JOY_DOWN | JOY_I));
    }

  return 0;
}

static
char
pong_keys(char key)
{
  switch(key)
    {
    case 0x1:
      return (joy(0) & JOY_UP);
    case 0x4:
      return (joy(0) & JOY_DOWN);
    case 0xC:
      return (joy(1) & JOY_UP);
    case 0xD:
      return (joy(1) & JOY_DOWN);
    }
}

static
char
blinky_keys(char key)
{
  switch(key)
    {
    case 0x1:
      return (joy(0) & JOY_I);
    case 0x3:
      return (joy(0) & JOY_UP);
    case 0x6:
      return (joy(0) & JOY_DOWN);
    case 0x7:
      return (joy(0) & JOY_LEFT);
    case 0x8:
      return (joy(0) & JOY_RIGHT);
    case 0xF:
      return (joy(0) & JOY_RIGHT);
    }

  return 0;
}

static
char
left_right_keys(char key)
{
  switch(key)
    {
    case 0x4:
      return (joy(0) & JOY_LEFT);
    case 0x6:
      return (joy(0) & JOY_RIGHT);
    }

  return 0;
}

static
char
tank_keys(char key)
{
  switch(key)
    {
    case 0x2:
      return (joy(0) & JOY_DOWN);
    case 0x4:
      return (joy(0) & JOY_LEFT);
    case 0x6:
      return (joy(0) & JOY_RIGHT);
    case 0x8:
      return (joy(0) & JOY_UP);
    case 0x5:
      return (joy(0) & JOY_I);
    }

  return 0;
}

static
char
spaceinvaders_keys(char key)
{
  switch(key)
    {
    case 0x4:
      return (joy(0) & JOY_LEFT);
    case 0x6:
      return (joy(0) & JOY_RIGHT);
    case 0x5:
      return (joy(0) & JOY_I);
    }

  return 0;
}

static
char
syzygy_keys(char key)
{
  switch(key)
    {
    case 0x3:
      return (joy(0) & JOY_UP);
    case 0x6:
      return (joy(0) & JOY_DOWN);
    case 0x7:
      return (joy(0) & JOY_LEFT);
    case 0x8:
      return (joy(0) & JOY_RIGHT);

    case 0xB:
      return (joy(0) & JOY_RUN);
    case 0xE:
      return (joy(0) & JOY_II);
    case 0xF:
      return (joy(0) & JOY_I);
    }

  return 0;
}

static
char
lunarlander_keys(char key)
{
  switch(key)
    {
    case 0x2:
      return (joy(0) & JOY_UP);
    case 0x4:
      return (joy(0) & JOY_LEFT);
    case 0x6:
      return (joy(0) & JOY_RIGHT);
    }
}

static
char
tetris_keys(char key)
{
  switch(key)
    {
    case 0x4:
      return (joy(0) & JOY_I);
    case 0x5:
      return (joy(0) & JOY_LEFT);
    case 0x6:
      return (joy(0) & JOY_RIGHT);
    case 0x7:
      return (joy(0) & JOY_DOWN);
    }
}

static
char
cave_keys(char key)
{
  switch(key)
    {
    case 0x2:
      return (joy(0) & JOY_UP);
    case 0x4:
      return (joy(0) & JOY_LEFT);
    case 0x6:
      return (joy(0) & JOY_RIGHT);
    case 0x8:
      return (joy(0) & JOY_DOWN);

    case 0xF:
      return (joy(0) & JOY_I);
    }
}

static
char
rocket_keys(char key)
{
  switch(key)
    {
    case 0x4:
      return (joy(0) & JOY_LEFT);
    case 0x6:
      return (joy(0) & JOY_RIGHT);

    case 0xB:
      return (joy(0) & JOY_I);
    }
}

char
wait_for_key()
{
  while(1)
    {
      switch(joy(0))
        {
        case JOY_UP_SEL:
          return 0x1;
        case JOY_UP_RUN:
          return 0x2;
        case JOY_UP_II:
          return 0x3;
        case JOY_UP_I:
          return 0xC;

        case JOY_LEFT_SEL:
          return 0x4;
        case JOY_LEFT_RUN:
          return 0x5;
        case JOY_LEFT_II:
          return 0x6;
        case JOY_LEFT_I:
          return 0xD;

        case JOY_RIGHT_SEL:
          return 0x7;
        case JOY_RIGHT_RUN:
          return 0x8;
        case JOY_RIGHT_II:
          return 0x9;
        case JOY_RIGHT_I:
          return 0xE;

        case JOY_DOWN_SEL:
          return 0xA;
        case JOY_DOWN_RUN:
          return 0x0;
        case JOY_DOWN_II:
          return 0xB;
        case JOY_DOWN_I:
          return 0xF;
        }
    }
}
