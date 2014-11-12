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

#define PSG_CTRL_ENABLED_FULL_VOL  0x9F
#define PSG_CTRL_ENABLED_MUTED     0x80
#define PSG_CTRL_DISABLED_FULL_VOL 0x1F
#define PSG_CTRL_DISABLED_MUTED    0x00

const unsigned char *psg_ch      = 0x800;
const unsigned char *psg_bal     = 0x801;
const unsigned char *psg_freqlo  = 0x802;
const unsigned char *psg_freqhi  = 0x803;
const unsigned char *psg_ctrl    = 0x804;
const unsigned char *psg_chbal   = 0x805;
const unsigned char *psg_data    = 0x806;
const unsigned char *psg_noise   = 0x807;
const unsigned char *psg_lfofreq = 0x808;
const unsigned char *psg_lfoctrl = 0x809;
const unsigned char sine_waveform[] =
  {
    0x0f, 0x12, 0x15, 0x17, 0x19, 0x1b, 0x1d, 0x1e,
    0x1e, 0x1e, 0x1d, 0x1b, 0x19, 0x17, 0x15, 0x12,
    0x0f, 0x0c, 0x09, 0x07, 0x05, 0x03, 0x01, 0x00,
    0x00, 0x00, 0x01, 0x03, 0x05, 0x07, 0x09, 0x0c
  };

void
psg_reset_waveform_index(void)
{
  *psg_ctrl = 0x40;
  *psg_ctrl = 0x00;
}

void
psg_load_waveform(char *waveform)
{
  char i;
  for(i = 0; i < 32; i++)
    *psg_data = *waveform++;
}
