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

#define X    (opcode.byte.high & 0x0F)
#define Y    (opcode.byte.low >> 4)
#define XY   ((opcode.word & 0x0FF0) >> 4)
#define KK   (opcode.byte.low)
#define K    (opcode.byte.low & 0x0F)
#define ADDR (opcode.word & 0x0FFF)

unsigned int  PC;
unsigned char SP;
unsigned int  I;
unsigned char v[16];
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

  done = 0;
  while(!done)
    done = chip8_process();

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
          break;

          /*
            00EE - RET
            Return from a subroutine.

            The interpreter sets the program counter to the address at the
            top of the stack, then subtracts 1 from the stack pointer.
          */
        case 0xEE:
          PC = STACK[--SP];
          break;

          /*
            00FD - EXIT
            Exit CHIP interpreter

            Super Chip-8 instruction to stop the interpreter.
           */
        case 0xFD:
          return 1;

          /*
            00FE - LOW
            Enable low res (64x32) mode
          */
        case 0xFE:
          break;

          /*
            00FF - HIGH
            Enable high res (128x64) mode
          */
        case 0xFF:
          break;

        default:
          print_invalidopcode(opcode.word);
          return 1;
        }
      break;

      /* 0x1nnn : jmp nnn : jump to address nnn */
    case 0x10:
      PC = ADDR;
      break;

      /*
        2nnn - CALL addr
        Call subroutine at nnn.

        The interpreter increments the stack pointer, then puts the current PC
        on the top of the stack. The PC is then set to nnn.
      */
    case 0x20:
      STACK[SP++] = PC;
      PC = ADDR;
      break;

      /*
        3xkk - SE Vx, byte
        Skip next instruction if Vx = kk.

        The interpreter compares register Vx to kk, and if they are
        equal, increments the program counter by 2.
      */
    case 0x30:
      if(v[X] == KK)
        PC += 2;
      break;

      /*
        4xkk - SNE Vx, byte
        Skip next instruction if Vx != kk.

        The interpreter compares register Vx to kk, and if they are
        not equal, increments the program counter by 2.
      */
    case 0x40:
      if(v[X] != KK)
        PC += 2;
      break;

    case 0x50:
      switch(opcode.byte.low & 0x0F)
        {
          /*
            5xy0 - SE Vx, Vy
            Skip next instruction if Vx = Vy.

            The interpreter compares register Vx to register Vy, and if
            they are equal, increments the program counter by 2.
          */
        case 0x00:
          if(v[X] == v[Y])
            PC += 2;
          break;

          /*
            5xy1 - SGT Vx, Vy
            Skip next instruction if Vx > Vy.

            The interpreter compares register Vx to register Vy, and if
            Vx > Vy, increments the program counter by 2.
          */
        case 0x01:
          if(v[X] > v[Y])
            PC += 2;
          break;

          /*
            5xy2 - SLT Vx, Vy
            Skip next instruction if Vx < Vy.

            The interpreter compares register Vx to register Vy, and if
            Vx < Vy, increments the program counter by 2.
          */
        case 0x02:
          if(v[X] < v[Y])
            PC += 2;
          break;

          /*
            5xy3 - SNE Vx, Vy
            Skip next instruction if Vx != Vy.

            The interpreter compares register Vx to register Vy, and if
            Vx != Vy, increments the program counter by 2.
          */
        case 0x03:
          if(v[X] != v[Y])
            PC += 2;
          break;

        default:
          print_invalidopcode(opcode.word);
          return 1;
        }
      break;

      /*
        6xkk - LD Vx, byte
        Set Vx = kk.

        The interpreter puts the value kk into register Vx.
      */
    case 0x60:
      v[X] = KK;
      break;

      /*
        7xkk - ADD Vx, byte
        Set Vx = Vx + kk.

        Adds the value kk to the value of register Vx, then stores the
        result in Vx.
      */
    case 0x70:
      v[X] += KK;
      break;

    case 0x80:
      switch(opcode.byte.low & 0x0F)
        {
          /*
            8xy0 - LD Vx, Vy
            Set Vx = Vy.

            Stores the value of register Vy in register Vx.
          */
        case 0x00:
          v[X] = v[Y];
          break;

          /*
            8xy1 - OR Vx, Vy
            Set Vx = Vx OR Vy.

            Performs a bitwise OR on the values of Vx and Vy, then stores
            the result in Vx. A bitwise OR compares the corrseponding bits
            from two values, and if either bit is 1, then the same bit in
            the result is also 1. Otherwise, it is 0.
          */
        case 0x01:
          v[X] |= v[Y];
          break;

          /*
            8xy2 - AND Vx, Vy
            Set Vx = Vx AND Vy.

            Performs a bitwise AND on the values of Vx and Vy, then stores the
            result in Vx. A bitwise AND compares the corrseponding bits from two
            values, and if both bits are 1, then the same bit in the result is
            also 1. Otherwise, it is 0.
          */
        case 0x02:
          v[X] &= v[Y];
          break;

          /*
            8xy3 - XOR Vx, Vy
            Set Vx = Vx XOR Vy.

            Performs a bitwise exclusive OR on the values of Vx and
            Vy, then stores the result in Vx. An exclusive OR
            compares the corrseponding bits from two values, and if
            the bits are not both the same, then the corresponding
            bit in the result is set to 1. Otherwise, it is 0.
          */
        case 0x03:
          v[X] ^= v[Y];
          break;

          /*
            8xy4 - ADD Vx, Vy
            Set Vx = Vx + Vy, set VF = carry.

            The values of Vx and Vy are added together. If the result is
            greater than 8 bits (i.e., > 255,) VF is set to 1, otherwise
            0. Only the lowest 8 bits of the result are kept, and stored
            in Vx.
          */
        case 0x04:
          v[0xF] = ((v[X] + v[Y]) > 0xFF);
          v[X] += v[Y];
          break;

          /*
            8xy5 - SUB Vx, Vy
            Set Vx = Vx - Vy, set VF = NOT borrow.

            If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is
            subtracted from Vx, and the results stored in Vx.
          */
        case 0x05:
          v[0xF] = (v[X] > v[Y]);
          v[X] -= v[Y];
          break;

          /*
            8xy6 - SHR Vx {, Vy}
            Originally: Set Vx = Vy SHR 1
            Today: Set Vx = Vx SHR 1

            If the least-significant bit of Vy is 1, then VF is set to 1,
            otherwise 0. Then Vy is shifted right by 1 and stored in Vx.
          */
        case 0x06:
          v[0xF] = (v[X] & 0x01);
          v[X]   = v[X] >> 1;
          break;

          /*
            8xy7 - SUBN Vx, Vy
            Set Vx = Vy - Vx, set VF = NOT borrow.

            If Vy > Vx, then VF is set to 1, otherwise 0. Then Vx is
            subtracted from Vy, and the results stored in Vx.
          */
        case 0x07:
          v[0xF] = (v[Y] > v[X]);
          v[X] = v[Y] - v[X];
          break;

          /*
            8xyE - SHL Vx {, Vy}
            Originally: Set Vx = Vy SHL 1
            Today: Set Vx = Vx SHL 1

            If the most-significant bit of Vx is 1, then VF is set to 1,
            otherwise to 0. Then Vx is multiplied by 2.
          */
        case 0x0E:
          v[0xF] = (v[X] & 0x80) && 1;
          v[X]   = v[X] << 1;
          break;

        default:
          print_invalidopcode(opcode.word);
          return 1;
        }
      break;

    case 0x90:
      switch(opcode.byte.low & 0x0F)
        {
          /*
            9xy0 - SNE Vx, Vy
            Skip next instruction if Vx != Vy.

            The values of Vx and Vy are compared, and if they are not equal, the
            program counter is increased by 2.
          */
        case 0x00:
          if(v[X] != v[Y])
            PC += 2;
          break;

          /*
            9xy1 - MUL Vx, Vy
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
          break;

          /*
            9xy2 - DIV Vx, Vy
            Set VF,VX = VX / VY

            Set VX equal to VX divided by VY where VF is the remainder.
          */
        case 0x02:
          v[X]   = v[X] / v[Y];
          v[0xF] = v[X] % v[Y];
          break;

          /*
            9xy3 - BCD VX, VY
            Convert VX, VY as a 16bit word to BCD at I

            Let VX, VY be treated as a 16bit word with VX the most
            significant part and convert to decimal; 5 decimal digits
            are stored at M(I), M(I+1), M(I+2), M(I+3), and M(I+4), I
            does not change.
          */
        case 0x03:
          bcd_convert_16bit(XY,&RAM[I]);
          break;

        default:
          print_invalidopcode(opcode.word);
          return 1;
        }
      break;

      /*
        Annn - LD I, addr
        Set I = nnn.

        The value of register I is set to nnn.
      */
    case 0xA0:
      I = ADDR;
      break;

      /*
        Bnnn - JP V0, addr
        Jump to location nnn + V0.

        The program counter is set to nnn plus the value of V0.
      */
    case 0xB0:
      PC = ADDR + (int)v[0];
      break;

      /*
        Cxkk - RND Vx, byte
        Set Vx = random byte AND kk.

        The interpreter generates a random number from 0 to 255, which
        is then ANDed with the value kk. The results are stored in
        Vx. See instruction 8xy2 for more information on AND.
      */
    case 0xC0:
      v[X] = ((char)rand() & KK);
      break;

    case 0xD0:
      /*
        Dxyn - DRW Vx, Vy, nibble
        Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.

        The interpreter reads n bytes from memory, starting at the address
        stored in I. These bytes are then displayed as sprites on screen
        at coordinates (Vx, Vy). Sprites are XORed onto the existing
        screen. If this causes any pixels to be erased, VF is set to 1,
        otherwise it is set to 0. If the sprite is positioned so part of
        it is outside the coordinates of the display, it wraps around to
        the opposite side of the screen. See instruction 8xy3 for more
        information on XOR, and section 2.4, Display, for more information
        on the Chip-8 screen and sprites.
      */
      v[0xF] = chip8_put_sprite(&RAM[I],v[X],v[Y],K);
      break;

    case 0xE0:
      switch(opcode.byte.low)
        {
          /*
            Ex9E - SKP Vx
            Skip next instruction if key with the value of Vx is pressed.

            Checks the keyboard, and if the key corresponding to the value of Vx
            is currently in the down position, PC is increased by 2.
          */
        case 0x9E:
          keymask |= (1 << v[X]);
          if(key_pressed(v[X]))
            PC += 2;
          break;

          /*
            ExA1 - SKNP Vx
            Skip next instruction if key with the value of Vx is not pressed.

            Checks the keyboard, and if the key corresponding to the value of Vx
            is currently in the up position, PC is increased by 2.
          */
        case 0xA1:
          keymask |= (1 << v[X]);
          if(!key_pressed(v[X]))
            PC += 2;
          break;

        default:
          print_invalidopcode(opcode.word);
          return 1;
        }
      break;

    case 0xF0:
      switch(opcode.byte.low)
        {
          /*
            Fx07 - LD Vx, DT
            Set Vx = delay timer value.

            The value of DT is placed into Vx.
          */
        case 0x07:
          v[X] = delay_timer;
          break;

          /*
            Fx0A - LD Vx, K
            Wait for a key press, store the value of the key in Vx.

            All execution stops until a key is pressed, then the value of that key is stored in Vx.
          */
        case 0x0A:
          v[X] = wait_for_key();
          break;

          /*
            Fx15 - LD DT, Vx
            Set delay timer = Vx.

            DT is set equal to the value of Vx.
          */
        case 0x15:
          delay_timer = v[X];
          break;

          /*
            Fx18 - LD ST, Vx
            Set sound timer = Vx.

            ST is set equal to the value of Vx.
          */
        case 0x18: /* 0xfr18 : ssound vr */
          sound_timer = v[X];
          if(sound_timer != 0)
            *psg_ctrl = PSG_CTRL_ENABLED_FULL_VOL;
          break;

          /*
            Fx1E - ADD I, Vx
            Set I = I + Vx.

            The values of I and Vx are added, and the results are stored in I.
          */
        case 0x1E:
          I = (I + v[X]);
          v[0xF] = (I > 0x0FFF);
          break;

          /*
            Fx29 - LD F, Vx
            Set I = location of sprite for digit Vx.

            The value of I is set to the location for the hexadecimal sprite
            corresponding to the value of Vx. See section 2.4, Display, for more
            information on the Chip-8 hexadecimal font.
          */
        case 0x29:
          I = ((int)v[X] * 5);
          break;

          /*
            Fx30 - LD HF, Vx
            Set I = location of sprite for large digit Vx.

            The value of I is set to the location for the hexadecimal sprite
            corresponding to the value of Vx. See section 2.4, Display, for more
            information on the Chip-8 hexadecimal font.
          */
        case 0x30:
          /* positioned after the 5x8 font */
          I = (((int)v[X] * 10) + (16 * 5));
          break;

          /*
            Fx33 - LD B, Vx
            Store BCD representation of Vx in memory locations I, I+1, and I+2.

            The interpreter takes the decimal value of Vx, and places the
            hundreds digit in memory at location in I, the tens digit at
            location I+1, and the ones digit at location I+2.
          */
        case 0x33:
          bcd_convert_8bit(v[X],&RAM[I]);
          break;

          /*
            Fx55 - LD [I], Vx
            Store registers V0 through Vx in memory starting at location I.

            The interpreter copies the values of registers V0 through Vx
            into memory, starting at the address in I.
          */
        case 0x55:
          memcpy(&RAM[I],&v[0],X+1);
          I += X + 1;
          break;

          /*
            Fx65 - LD Vx, [I]
            Read registers V0 through Vx from memory starting at location I.

            The interpreter reads values from memory starting at location
            I into registers V0 through Vx.
          */
        case 0x65:
          memcpy(&v[0],&RAM[I],X+1);
          I += X + 1;
          break;

          /*
            Fx75 - DISP Vx
            Display the value of Vx on the hex display.

            Additional "F" instruction added for the Elf
          */
        case 0x75:
          break;

          /*
            Fx94 - LD ASCII, Vx
            Load I with location of ASCII char
          */
        case 0x94:
          break;

        default:
          print_invalidopcode(opcode.word);
          return 1;
        }
      break;

    default:
      print_invalidopcode(opcode.word);
      return 1;
    }

  return 0;
}

void
print_invalidopcode(int opc)
{
  put_string("Unknown opcode:", 0, 27);
  put_hex(opc, 4, 17, 27);
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
