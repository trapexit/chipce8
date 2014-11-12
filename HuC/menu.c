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

#include "fmemcpy.c"
#include "roms.c"

#define PER_PAGE 26

int
menu(void)
{
  int idx;
  int page;
  int prevpage;
  int pages;
  int num_of_roms;
  unsigned char x, y, i;
  unsigned char joypad;
  unsigned char prevjoypad;

  setup_screen(384);

  num_of_roms = sizeof(roms) / sizeof(char*);
  pages = ((num_of_roms + (PER_PAGE-1)) / PER_PAGE);

  idx = 0;
  while(1)
    {
      page = (idx / PER_PAGE);
      i = (page * PER_PAGE);
      x = 1;
      y = 1;

      if(page != prevpage)
        cls();
      put_string("Page:", 0, 0);
      put_number(page+1, 1, 6, 0);
      put_char('/', 7, 0);
      put_number(pages, 1, 8, 0);
      for(y = 1; y <= PER_PAGE && i < num_of_roms; y++)
        {
          put_char(idx == i ? '>' : ' ', x-1, y);
          put_string(roms[i],x,y);
          i++;
        }

      vsync();

      prevpage = page;

      joypad = joy(0);
      if(joypad & JOY_I)
        return idx;

      if((joypad & 0xF0) == (prevjoypad & 0xF0))
        continue;

      if((joypad & JOY_UP))
        idx -= (joypad & JOY_II) ? 5 : 1;
      else if((joypad & JOY_DOWN))
        idx += (joypad & JOY_II) ? 5 : 1;
      else if(joypad & JOY_LEFT)
        idx -= PER_PAGE;
      else if(joypad & JOY_RIGHT)
        idx += PER_PAGE;

      if(idx < 0)
        idx = 0;
      else if(idx >= num_of_roms)
        idx = num_of_roms - 1;

      prevjoypad = joypad;
    }
}
