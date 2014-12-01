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

#include "psg.c"
#include "font.c"
#include "joypad.c"
#include "sprite.c"
#include "bcd.c"

#define X   (opcode.byte.high & 0x0F)
#define Y   (opcode.byte.low >> 4)
#define N   (opcode.byte.low & 0x0F)
#define NN  (opcode.byte.low)
#define NNN (opcode.word & 0x0FFF)

#define SUCCESS            0
#define UNSUPPORTED_OPCODE 1
#define INVALID_OPCODE     2

unsigned int  PC;
unsigned char SP;
unsigned int  I;
unsigned char v[16];
unsigned char v48[8];
unsigned char RAM[4096];
unsigned int  STACK[16];
unsigned char delay_timer;
unsigned char sound_timer;
unsigned int  keymask;

union
{
  unsigned int word;

  struct
  {
    unsigned char low;
    unsigned char high;
  } byte;
} opcode;

void
chip8_psg_init()
{
  *psg_bal    = 0xFF;
  *psg_ch     = 0x00;
  *psg_freqlo = 0xFF;
  *psg_freqhi = 0x00;
  *psg_chbal  = 0xFF;
  psg_reset_waveform_index();
  psg_load_waveform(sine_waveform);
}

static
void
chip8_vsync_hook(void) __mapcall __irq
{
  if(delay_timer != 0)
    delay_timer--;

  if(sound_timer != 0)
    {
      sound_timer--;
      if(sound_timer == 0)
        *psg_ctrl = PSG_CTRL_ENABLED_MUTED;
    }
}

void
chip8_init()
{
  memset(v,0,sizeof(v));
  memset(STACK,0,sizeof(STACK));
  memset(RAM,0,sizeof(RAM));

  I           = 0;
  PC          = 0x200;
  SP          = 0x00;
  delay_timer = 0;
  sound_timer = 0;
  opcode.word = 0x0000;
  keymask     = 0;

  chip8_font_init(&RAM[0]);

  chip8_psg_init();
}

void
chip8(void)
{
  irq_add_vsync_handler(chip8_vsync_hook);
  irq_enable_user(IRQ_VSYNC);

  chip8_loop();
}

static
void
chip8_loop()
{
  char done;

  done = SUCCESS;
  while(!done)
    done = chip8_process();

  switch(done)
    {
    case UNSUPPORTED_OPCODE:
      print_unsupported_opcode(opcode.word);
      break;

    case INVALID_OPCODE:
      print_invalid_opcode(opcode.word);
      break;
    }

  vsync(60 * 3);
}

static
void
chip8_process()
{
  opcode.byte.high = RAM[PC++];
  opcode.byte.low  = RAM[PC++];

  switch(opcode.byte.high & 0xF0)
    {
    case 0x00:
      switch(opcode.byte.low)
        {
          /*
            00E0 - CLS
            Clear the display.
          */
        case 0xE0:
          gfx_clear(0x1000);
          return SUCCESS;

          /*
            00EE - RET
            Return from a subroutine.

            The interpreter sets the program counter to the address at
            the top of the stack, then subtracts 1 from the stack
            pointer.
          */
        case 0xEE:
          PC = STACK[--SP];
          return SUCCESS;

          /*
            00FB - SCR
            Scroll display right

            SCHIP-8 instruction to scroll display 4 pixels to the
            right.
          */
        case 0xFB:
          return UNSUPPORTED_OPCODE;

          /*
            00FC - SCL
            Scroll display left

            SCHIP-8 instruction to scroll display 4 pixels to the
            left.
          */
        case 0xFC:
          return UNSUPPORTED_OPCODE;

          /*
            00FD - EXIT
            Exit CHIP interpreter

            SCHIP-8 instruction to stop the interpreter.
           */
        case 0xFD:
          return UNSUPPORTED_OPCODE;

          /*
            00FE - LOW
            Enable low res (64x32) mode

            SCHIP-8 instruction to enable default / low res mode.
          */
        case 0xFE:
          return UNSUPPORTED_OPCODE;

          /*
            00FF - HIGH
            Enable high res (128x64) mode

            SCHIP-8 instruction to enable high res mode.
          */
        case 0xFF:
          return UNSUPPORTED_OPCODE;

        default:
          return INVALID_OPCODE;
        }

      /* 0x1nnn : jmp nnn : jump to address nnn */
    case 0x10:
      PC = NNN;
      return SUCCESS;

      /*
        2nnn - CALL addr
        Call subroutine at nnn.

        The interpreter increments the stack pointer, then puts the current PC
        on the top of the stack. The PC is then set to nnn.
      */
    case 0x20:
      STACK[SP++] = PC;
      PC = NNN;
      return SUCCESS;

      /*
        3xNN - SE VX, byte
        Skip next instruction if VX = NN.

        The interpreter compares register VX to NN, and if they are
        equal, increments the program counter by 2.
      */
    case 0x30:
      if(v[X] == NN)
        PC += 2;
      return SUCCESS;

      /*
        4xNN - SNE VX, byte
        Skip next instruction if VX != NN.

        The interpreter compares register VX to NN, and if they are
        not equal, increments the program counter by 2.
      */
    case 0x40:
      if(v[X] != NN)
        PC += 2;
      return SUCCESS;

    case 0x50:
      switch(opcode.byte.low & 0x0F)
        {
          /*
            5XY0 - SE VX, VY
            Skip next instruction if VX = VY.

            The interpreter compares register VX to register VY, and if
            they are equal, increments the program counter by 2.
          */
        case 0x00:
          if(v[X] == v[Y])
            PC += 2;
          return SUCCESS;

          /*
            5XY1 - SGT VX, VY
            Skip next instruction if VX > VY.

            The interpreter compares register VX to register VY, and if
            VX > VY, increments the program counter by 2.
          */
        case 0x01:
          if(v[X] > v[Y])
            PC += 2;
          return SUCCESS;

          /*
            5XY2 - SLT VX, VY
            Skip next instruction if VX < VY.

            The interpreter compares register VX to register VY, and if
            VX < VY, increments the program counter by 2.
          */
        case 0x02:
          if(v[X] < v[Y])
            PC += 2;
          return SUCCESS;

          /*
            5XY3 - SNE VX, VY
            Skip next instruction if VX != VY.

            The interpreter compares register VX to register VY, and if
            VX != VY, increments the program counter by 2.
          */
        case 0x03:
          if(v[X] != v[Y])
            PC += 2;
          return SUCCESS;

        default:
          return INVALID_OPCODE;
        }

      /*
        6XNN - LD VX, NN
        Set VX = NN.

        The interpreter puts the value NN into register VX.
      */
    case 0x60:
      v[X] = NN;
      return SUCCESS;

      /*
        7xNN - ADD VX, byte
        Set VX = VX + NN.

        Adds the value NN to the value of register VX, then stores the
        result in VX.
      */
    case 0x70:
      v[X] += NN;
      return SUCCESS;

    case 0x80:
      switch(opcode.byte.low & 0x0F)
        {
          /*
            8XY0 - LD VX, VY
            Set VX = VY.

            Stores the value of register VY in register VX.
          */
        case 0x00:
          v[X] = v[Y];
          return SUCCESS;

          /*
            8XY1 - OR VX, VY
            Set VX = VX OR VY.

            Performs a bitwise OR on the values of VX and VY, then stores
            the result in VX. A bitwise OR compares the corrseponding bits
            from two values, and if either bit is 1, then the same bit in
            the result is also 1. Otherwise, it is 0.
          */
        case 0x01:
          v[X] |= v[Y];
          return SUCCESS;

          /*
            8XY2 - AND VX, VY
            Set VX = VX AND VY.

            Performs a bitwise AND on the values of VX and VY, then stores the
            result in VX. A bitwise AND compares the corrseponding bits from two
            values, and if both bits are 1, then the same bit in the result is
            also 1. Otherwise, it is 0.
          */
        case 0x02:
          v[X] &= v[Y];
          return SUCCESS;

          /*
            8XY3 - XOR VX, VY
            Set VX = VX XOR VY.

            Performs a bitwise exclusive OR on the values of VX and
            VY, then stores the result in VX. An exclusive OR
            compares the corrseponding bits from two values, and if
            the bits are not both the same, then the corresponding
            bit in the result is set to 1. Otherwise, it is 0.
          */
        case 0x03:
          v[X] ^= v[Y];
          return SUCCESS;

          /*
            8XY4 - ADD VX, VY
            Set VX = VX + VY, set VF = carry.

            The values of VX and VY are added together. If the result is
            greater than 8 bits (i.e., > 255,) VF is set to 1, otherwise
            0. Only the lowest 8 bits of the result are kept, and stored
            in VX.
          */
        case 0x04:
          v[0xF] = ((v[X] + v[Y]) < v[X]);
          v[X] += v[Y];
          return SUCCESS;

          /*
            8XY5 - SUB VX, VY
            Set VX = VX - VY, set VF = NOT borrow.

            If VX > VY, then VF is set to 1, otherwise 0. Then VY is
            subtracted from VX, and the results stored in VX.
          */
        case 0x05:
          v[0xF] = (v[X] >= v[Y]);
          v[X] -= v[Y];
          return SUCCESS;

          /*
            8XY6 - SHR VX {, VY}
            Originally: Set VX = VY SHR 1
            Today: Set VX = VX SHR 1

            If the least-significant bit of VY is 1, then VF is set to 1,
            otherwise 0. Then VY is shifted right by 1 and stored in VX.
          */
        case 0x06:
          v[0xF] = (v[X] & 0x01);
          v[X] >>= 1;
          return SUCCESS;

          /*
            8XY7 - SUBN VX, VY
            Set VX = VY - VX, set VF = NOT borrow.

            If VY > VX, then VF is set to 1, otherwise 0. Then VX is
            subtracted from VY, and the results stored in VX.
          */
        case 0x07:
          v[0xF] = (v[Y] >= v[X]);
          v[X]   = (v[Y] - v[X]);
          return SUCCESS;

          /*
            8XYE - SHL VX {, VY}
            Originally: Set VX = VY SHL 1
            Today: Set VX = VX SHL 1

            If the most-significant bit of VX is 1, then VF is set to 1,
            otherwise to 0. Then VX is multiplied by 2.
          */
        case 0x0E:
          v[0xF] = ((v[X] & 0x80) ? 1 : 0);
          v[X] <<= 1;
          return SUCCESS;

        default:
          return INVALID_OPCODE;
        }

    case 0x90:
      switch(opcode.byte.low & 0x0F)
        {
          /*
            9XY0 - SNE VX, VY
            Skip next instruction if VX != VY.

            The values of VX and VY are compared, and if they are not equal, the
            program counter is increased by 2.
          */
        case 0x00:
          if(v[X] != v[Y])
            PC += 2;
          return SUCCESS;

          /*
            9XY1 - MUL VX, VY
            Set VF,VX = VX * VY

            Set VF, VX equal to VX times VY where VF is the most
            significant part of a 16bit word.
          */
        case 0x01:
          {
            struct {char l; char h;} result;

            result = v[X] * v[Y];
            v[0xF] = result.h;
            v[X]   = result.l;
          }
          return SUCCESS;

          /*
            9XY2 - DIV VX, VY
            Set VF,VX = VX / VY

            Set VX equal to VX divided by VY where VF is the remainder.
          */
        case 0x02:
          v[X]   = v[X] / v[Y];
          v[0xF] = v[X] % v[Y];
          return SUCCESS;

          /*
            9XY3 - BCD VX, VY
            Convert VX, VY as a 16bit word to BCD at I

            Let VX, VY be treated as a 16bit word with VX the most
            significant part and convert to decimal; 5 decimal digits
            are stored at M(I), M(I+1), M(I+2), M(I+3), and M(I+4), I
            does not change.
          */
        case 0x03:
          {
            struct {char l; char h;} tmp;

            tmp.h = v[X];
            tmp.l = v[Y];

            bcd_convert_16bit(tmp,&RAM[I]);
          }
          return SUCCESS;

        default:
          return INVALID_OPCODE;
        }

      /*
        ANNN - LD I, addr
        Set I = NNN.

        The value of register I is set to nnn.
      */
    case 0xA0:
      I = NNN;
      return SUCCESS;

      /*
        BNNN - JP V0, addr
        Jump to location NNN + V0.

        The program counter is set to NNN plus the value of V0.
      */
    case 0xB0:
      PC = NNN + (int)v[0];
      return SUCCESS;

      /*
        CXNN - RND VX, NN
        Set VX = random byte AND NN.

        The interpreter generates a random number from 0 to 255, which
        is then ANDed with the value NN. The results are stored in
        VX.
      */
    case 0xC0:
      v[X] = ((char)rand() & NN);
      return SUCCESS;

    case 0xD0:
      /*
        DXYN - DRW VX, VY, N
        Display N-byte sprite starting at memory location I at (VX, VY), set VF = collision.

        The interpreter reads n bytes from memory, starting at the address
        stored in I. These bytes are then displayed as sprites on screen
        at coordinates (VX, VY). Sprites are XORed onto the existing
        screen. If this causes any pixels to be erased, VF is set to 1,
        otherwise it is set to 0. If the sprite is positioned so part of
        it is outside the coordinates of the display, it wraps around to
        the opposite side of the screen. See instruction 8XY3 for more
        information on XOR, and section 2.4, Display, for more information
        on the Chip-8 screen and sprites.
      */
      v[0xF] = chip8_put_sprite(&RAM[I],v[X],v[Y],N);
      return SUCCESS;

    case 0xE0:
      switch(opcode.byte.low)
        {
          /*
            Ex9E - SKP VX
            Skip next instruction if key with the value of VX is pressed.

            Checks the keyboard, and if the key corresponding to the value of VX
            is currently in the down position, PC is increased by 2.
          */
        case 0x9E:
          keymask |= (1 << v[X]);
          if(key_pressed(v[X]))
            PC += 2;
          return SUCCESS;

          /*
            ExA1 - SKNP VX
            Skip next instruction if key with the value of VX is not pressed.

            Checks the keyboard, and if the key corresponding to the value of VX
            is currently in the up position, PC is increased by 2.
          */
        case 0xA1:
          keymask |= (1 << v[X]);
          if(!key_pressed(v[X]))
            PC += 2;
          return SUCCESS;

        default:
          return INVALID_OPCODE;
        }

    case 0xF0:
      switch(opcode.byte.low)
        {
          /*
            FX07 - LD VX, DT
            Set VX = delay timer value.

            The value of DT is placed into VX.
          */
        case 0x07:
          v[X] = delay_timer;
          return SUCCESS;

          /*
            FX0A - LD VX, N
            Wait for a key press, store the value of the key in VX.

            All execution stops until a key is pressed, then the value of that key is stored in VX.
          */
        case 0x0A:
          v[X] = wait_for_key();
          return SUCCESS;

          /*
            FX15 - LD DT, VX
            Set delay timer = VX.

            DT is set equal to the value of VX.
          */
        case 0x15:
          delay_timer = v[X];
          return SUCCESS;

          /*
            FX18 - LD ST, VX
            Set sound timer = VX.

            ST is set equal to the value of VX.
          */
        case 0x18:
          sound_timer = v[X];
          if(sound_timer != 0)
            *psg_ctrl = PSG_CTRL_ENABLED_FULL_VOL;
          return SUCCESS;

          /*
            FX1E - ADD I, VX
            Set I = I + VX.

            The values of I and VX are added, and the results are stored in I.
          */
        case 0x1E:
          I += v[X];
          v[0xF] = (I > 0x0FFF);
          I &= 0x0FFF;
          return SUCCESS;

          /*
            FX29 - LD F, VX
            Set I = location of sprite for digit VX.

            The value of I is set to the location for the hexadecimal sprite
            corresponding to the value of VX.
          */
        case 0x29:
          I = chip8_font_8x5_loc(v[X]);
          return SUCCESS;

          /*
            FX30 - LD HF, VX
            Set I = location of sprite for large digit VX.

            The value of I is set to the location for the hexadecimal sprite
            corresponding to the value of VX.
          */
        case 0x30:
          I = chip8_font_16x10_loc(v[X]);
          return SUCCESS;

          /*
            FX33 - LD B, VX
            Store BCD representation of VX in memory locations I, I+1, and I+2.

            The interpreter takes the decimal value of VX, and places the
            hundreds digit in memory at location in I, the tens digit at
            location I+1, and the ones digit at location I+2.
          */
        case 0x33:
          bcd_convert_8bit(v[X],&RAM[I]);
          return SUCCESS;

          /*
            FX55 - LD [I], VX
            Store registers V0 through VX in memory starting at location I.

            The interpreter copies the values of registers V0 through VX
            into memory, starting at the address in I.
          */
        case 0x55:
          memcpy(&RAM[I],&v[0],X+1);
          return SUCCESS;

          /*
            FX65 - LD VX, [I]
            Read registers V0 through VX from memory starting at location I.

            The interpreter reads values from memory starting at location
            I into registers V0 through VX.
          */
        case 0x65:
          memcpy(&v[0],&RAM[I],X+1);
          return SUCCESS;

          /*
            FX75 - DISP VX
            CHIP-8E Display the value of VX on the hex display.

            FX75 - LD R, VX
            SCHIP-8 Store V0 through VX to HP-48 RPL user flags (X <= 7).
          */
        case 0x75:
          memcpy(&v[0],&v48[0],((X+1) & 0x03));
          return SUCCESS;

          /*
            FX85 - LD VX, R
            SCHIP-8 Store HP-48 RPL user flags in V0 through VX (X <= 7).
          */
        case 0x85:
          memcpy(&v48[0],&v[0],((X+1) & 0x03));
          return SUCCESS;

          /*
            FX94 - LD ASCII, VX
            Load I with location of ASCII char
          */
        case 0x94:
          return UNSUPPORTED_OPCODE;

        default:
          return INVALID_OPCODE;
        }

    default:
      return INVALID_OPCODE;
    }

  return INVALID_OPCODE;
}

void
print_invalid_opcode(int opc)
{
  put_string("Unknown opcode:", 0, 27);
  put_hex(opc, 4, 17, 27);
}

void
print_unsupported_opcode(int opc)
{
  put_string("Unsupported opcode:", 0, 27);
  put_hex(opc, 4, 22, 27);
}

void
print_keymask()
{
  put_hex((keymask >> 0x1) & 0x01,1,20,0);
  put_hex((keymask >> 0x2) & 0x01,1,21,0);
  put_hex((keymask >> 0x3) & 0x01,1,22,0);
  put_hex((keymask >> 0xC) & 0x01,1,23,0);

  put_hex((keymask >> 0x4) & 0x01,1,20,1);
  put_hex((keymask >> 0x5) & 0x01,1,21,1);
  put_hex((keymask >> 0x6) & 0x01,1,22,1);
  put_hex((keymask >> 0xD) & 0x01,1,23,1);

  put_hex((keymask >> 0x7) & 0x01,1,20,2);
  put_hex((keymask >> 0x8) & 0x01,1,21,2);
  put_hex((keymask >> 0x9) & 0x01,1,22,2);
  put_hex((keymask >> 0xE) & 0x01,1,23,2);

  put_hex((keymask >> 0xA) & 0x01,1,20,3);
  put_hex((keymask >> 0x0) & 0x01,1,21,3);
  put_hex((keymask >> 0xB) & 0x01,1,22,3);
  put_hex((keymask >> 0xF) & 0x01,1,23,3);
}

void
print_details()
{
  char i,b,e;

  i = 0;
  put_string("PC: ", 20, i);
  put_hex(PC, 4, 23, i++);
  put_string("OP:", 20, i);
  put_hex(opcode.word, 4, 23, i++);
  put_string("DT:", 20, i);
  put_hex(delay_timer, 2, 23, i++);
  put_string("ST:", 20, i);
  put_hex(sound_timer, 2, 23, i++);
  put_string("SP: ", 20, i);
  put_hex(SP,4,23, i++);

  b = i;
  e = i + (sizeof(STACK)>>1);
  for(; i < e; i++)
    {
      char c;
      char offset;

      offset = i - b;
      if(SP == offset)
        c = '>';
      else
        c = ' ';
      put_char(c,22,i);
      put_hex(STACK[offset],4,23,i);
    }
}
