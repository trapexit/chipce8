# chipce8

CHIP-8 emulator for the PC Engine / TurboGrafx-16

## Devices
### CHIP-8

#### Description
From [Wikipedia](http://en.wikipedia.org/wiki/CHIP-8):

> CHIP-8 is an interpreted programming language, developed by Joseph Weisbecker. It was initially used on the COSMAC VIP and Telmac 1800 8-bit microcomputers in the mid-1970s. CHIP-8 programs are run on a CHIP-8 virtual machine. It was made to allow video games to be more easily programmed for said computers.
>
>Roughly twenty years after CHIP-8 was introduced, derived interpreters appeared for some models of graphing calculators (from the late 1980s onward, these handheld devices in many ways have more computing power than most mid-1970s microcomputers for hobbyists).
>
>An active community of users and developers existed in the late 1970s, beginning with ARESCO's "VIPer" newsletter whose first three issues revealed the machine code behind the CHIP-8 interpreter.

#### Technical Specifications
* 4KB RAM
* 1bit 64x32 pixel display
* 16 8bit data registers
* 16bit address register
* 16 level stack
* Delay timer (counts down at 60Hz)
* Sound timer (counts down at 60Hz)
* 16 key (hex) keyboard

### PC Engine / TurboGrafx-16

#### Description
From [Wikipedia](http://en.wikipedia.org/wiki/TurboGrafx-16):

>The TurboGrafx-16 Entertainment SuperSystem, originally known in Japan as the PC Engine (PCエンジン Pī Shī Enjin?), is a home video game console joint-developed by Hudson Soft and NEC, released in Japan on October 30, 1987, and in the United States on August 29, 1989. It was the first console released in the 16-bit era, albeit still utilizing an 8-bit CPU. Originally intended to compete with the Nintendo Entertainment System (NES), it ended up competing against the likes of the Sega Mega Drive/Genesis, Super Famicom/Super Nintendo, and even the Neo Geo AES.
>
>The TurboGrafx-16 has an 8-bit CPU and a dual 16-bit GPU; and is capable of displaying 482 colors simultaneously, out of 512. With dimensions of 14 cm×14 cm×3.8 cm (5.5 in×5.5 in×1.5 in), the NEC PC Engine once held the record for the world's smallest game console ever made.
>
>In the United Kingdom, Telegames released a slightly altered version of the American model simply as the TurboGrafx around 1990 in extremely limited quantities. Although there was no full-scale PAL region release of the system, imported PC Engine consoles were largely available in France and Benelux through major retailers thanks to the unlicensed importer Sodipeng (Société de Distribution de la PC Engine, a subsidiary of Guillemot International).
>
>Two major revisions, the PC Engine SuperGrafx and the TurboDuo, were released in 1989 and 1991, respectively. The entire series was succeeded by the PC-FX in 1994, which was only released in Japan.

#### Technical Specifications
* CPU: [HuC6280](http://en.wikipedia.org/wiki/Hudson_Soft_HuC6280) running at 1.79MHz or 7.16MHz
* Memory: 21bit addressable (2MB) via 16bit address bus with a MMU
* RAM: 8KB base, +64KB w/ CD-ROM, +192KB w/ System Card 3.00, +2048KB w/ Arcade Card
* Audio: 6 channels (4 waveform channels + 2 white noise channels)
* Graphics:
 * Resolution: 256x224 - 565x242 (including overscan)
 * VRAM: 64KB
 * Tiles: 1 layer, 8x8 pixels, 16 4bit or 32 2bit palettes
 * Sprites: 64 sprites, 16x16 - 32x64 pixels, 256 sprite pixels per scanline, 16 4bit palettes
 * Colors: 512 (9bit), maximum of 482 onscreen (due to dups & transparency)
* Input: up to 5 12 button joypads

## Emulator Details
### Core
There isn't much to say with regard to the core. CHIP-8 was designed for an [8-bit processor](http://en.wikipedia.org/wiki/RCA_1802) and is rather simple to implement in just about any language or platform. The core CHIP-8 opcodes are supported as well as some SCHIP-8 and extentions found in Paul C. Moews' *[Programs for the COSMAC ELF Interpreters](http://mattmik.com/documents.html)*.

### Graphics
The CHIP-8 uses a 1-bit 64x32 pixel [packed pixel](http://en.wikipedia.org/wiki/Packed_pixel) display. The PC Engine on the otherhand uses a [planar](http://patpend.net/technical/tg16/vdcdox.txt) sprite and tile system with a resolution up to 565x242 pixels and 9bit color.

If the PC Engine supported a resolution of 512x256 then we'd be able to map CHIP-8 pixels to all white or all black tiles. The closest we're able to get to that is 512x224 leaving us with a CHIP-8 pixel being 8x7 PC Engine pixels. Slightly lopsided but not distractingly so. Given the layout of pixel data in VRAM this works out reasonably well.

The 4 bits per pixel are stored across 4 bitplanes. 16 bytes (2 at a time) store bit 0 and 1 of 8 x 8 pixels for the tile. The following 16 bytes store bit 2 and 3. Since we are using all 8 horizontal pixels in the tile to represent 1 CHIP-8 pixel we can easily address the location in VRAM representing that line of 8 pixels. By setting the palette for all tiles to 0 = black and 1 through 15 = white we can simply set the first bitplane to 1 to turn the row white. This keeps the cost of drawling down significantly vs other methods.

Since tiles are stored left to right, top to bottom in memory we need to accomidate for when we reach the boundry of a tile. Once hit we simply jump to the next tile by adding the number of bytes per line of tiles.

Much of this can be precalculated further speeding up drawling. Such as the beginning of each CHIP-8 pixel row and where to jump to when crossing boundries.

### Sound & Delay Timers
CHIP-8 has only monotone sound therefore any sound can be generated while the sound timer is active. Since both timers count down at 60Hz we tie it to the vsync IRQ callback. It decrements both counters as well as disables sound should it reach 0. Enabling of sound is done when the sound timer is set to non-zero.

### Keyboard to joypad mapping
#### Original CHIP-8 keyboard
```
+---------------+
| 1 | 2 | 3 | C |
+---+---+---+---+
| 4 | 5 | 6 | D |
+---+---+---+---+
| 7 | 8 | 9 | E |
+---+---+---+---+
| A | 0 | B | F |
+---------------+
```

#### PC Engine joypad
```
+----------------------+
|  U  |         |      |
| L R | SEL RUN | II I |
|  D  |         |      |
+----------------------+
```

#### How it works
There are twice as many buttons on the CHIP-8 making 1 to 1 mapping impossible and they are arranged in a way that makes general mapping impractical. Rather than hardcoding the keybindings for each rom [chipce8](http://github.com/trapexit/chipce8) has a simple algo to provide for some common layouts and fall back to a 4 way shift key (multiplexed) layout which will at least allow all keys to be pressed should it not find a predefined layout.

It works as follows: as the CHIP-8 software queries for a key (SKP Vx or SKNP Vx) that key is placed into a 16bit mask representing all 16 CHIP-8 keys. That mask is checked each call to SKP and SKNP and if matched will apply the mapping for that mask.

#### Mappings
* single button -> I
* 4, 6 -> LEFT, RIGHT
* 2, 4, 6 -> UP, LEFT, RIGHT
* 1, 4, C, D -> P1: UP, DOWN; P2: UP, DOWN
* 1, 7, 8, 3, 6 -> I, LEFT, RIGHT, UP, DOWN
* 2, 4, 5, 6, 8 -> UP, LEFT, I, RIGHT, DOWN
* unknown:
  * 1 2 3 C -> UP    + SEL, RUN, II, I
  * 4 5 6 D -> LEFT  + SEL, RUN, II, I
  * 7 8 9 E -> RIGHT + SEL, RUN, II, I
  * A 0 B F -> DOWN  + SEL, RUN, II, I

```
FEDC BA98 7654 3210
0000 0000 0001 0010 = 0x0012 = PONG_1P
0000 0000 0101 0000 = 0x0050 = LEFT_RIGHT
0000 0000 0101 0100 = 0x0054 = LUNARLANDER
0000 0000 0111 0000 = 0x0070 = SPACEINVADERS
0000 0000 1111 0000 = 0x00F0 = TETRIS
0000 0001 0111 0100 = 0x0174 = TANK
0000 0001 1100 1010 = 0x01CA = BLINKY0
0000 1000 0101 0000 = 0x0850 = ROCKET
0011 0000 0001 0010 = 0x3012 = PONG_2P
1000 0001 0101 0100 = 0x8154 = CAVE
1000 0001 1100 1010 = 0x81CA = BLINKY1
1100 0000 0000 0000 = 0xC000 = SYZYGY_SELECT
1100 0001 1100 1000 = 0xC1C8 = SYZYGY0
1100 1001 1100 1000 = 0xC9C8 = SYZYGY1
```

#### Hardware Keypad
It wouldn't be too difficult to design a physical hex keypad to use with the PC Engine. The 2 write pins on the joypad port could be used to select the row via a multiplexer and then the row read via the 4 read pins.

### HuC vs. assembly
[chipce8](http://github.com/trapexit/chipce8) will be available in both [HuC](http://github.com/trapexit/huc) as well as [assembly](http://github.com/trapexit/pceas). Should be useful for those wishing to learn how to code in either language.
