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

static char collision;

static const int  *videoram   = 0x0002;
static const char *videoram_l = 0x0002;
static const char *videoram_h = 0x0003;
const unsigned int yaddr[] =
  {
    0x1000, 0x100F, 0x140E, 0x180D, 0x1C0C, 0x200B, 0x240A, 0x2809,
    0x2C00, 0x2C0F, 0x300E, 0x340D, 0x380C, 0x3C0B, 0x400A, 0x4409,
    0x4800, 0x480F, 0x4C0E, 0x500D, 0x540C, 0x580B, 0x5C0A, 0x6009,
    0x6400, 0x640F, 0x680E, 0x6C0D, 0x700C, 0x740B, 0x780A, 0x7C09
  };

char
chip8_put_sprite(char *sprite,
                 char  x,
                 char  y,
                 char  s)
{
  static char i;
  static char pixels;
  static int  baseaddr;

  collision = 0;
  for(i = 0; i < s; i++)
    {
      pixels   = *sprite++;
      baseaddr = yaddr[(y++ & 0x1F)];

      setpixel((baseaddr + (((x+0) & 0x3f) << 4)), (pixels & 0x80));
      setpixel((baseaddr + (((x+1) & 0x3f) << 4)), (pixels & 0x40));
      setpixel((baseaddr + (((x+2) & 0x3f) << 4)), (pixels & 0x20));
      setpixel((baseaddr + (((x+3) & 0x3f) << 4)), (pixels & 0x10));
      setpixel((baseaddr + (((x+4) & 0x3f) << 4)), (pixels & 0x08));
      setpixel((baseaddr + (((x+5) & 0x3f) << 4)), (pixels & 0x04));
      setpixel((baseaddr + (((x+6) & 0x3f) << 4)), (pixels & 0x02));
      setpixel((baseaddr + (((x+7) & 0x3f) << 4)), (pixels & 0x01));
    }

  return collision;
}

static
void
setpixel(const int addr,
         char      val)
{
  static char pixel;

  vreg(0x00);
  *videoram = addr;
  vreg(0x01);
  *videoram = addr;
  vreg(0x02);

  val   = !!val;
  pixel = !!*videoram_l;

  collision |= val & pixel;
  pixel      = val ^ pixel ? 0xFF : 0x00;

  *videoram_l = pixel;
  switch(addr & 0x07)
    {
    default:
    case 0:
    case 1:
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      break;

    case 2:
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      vreg(0x00,addr+1022);
      vreg(0x02);
      *videoram_h = 0x00;
      break;

    case 3:
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      vreg(0x00,addr+1021);
      vreg(0x02);
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      break;

    case 4:
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      vreg(0x00,addr+1020);
      vreg(0x02);
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      break;

    case 5:
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      vreg(0x00,addr+1019);
      vreg(0x02);
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      break;

    case 6:
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      vreg(0x00,addr+1018);
      vreg(0x02);
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      break;

    case 7:
      *videoram_h = 0x00;
      vreg(0x00,addr+1017);
      vreg(0x02);
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      *videoram_h = 0x00;
      break;
    }
}
