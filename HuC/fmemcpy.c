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

#pragma fastcall fmemcpy(word di, farptr _fbank:_fptr, word acc)

#asm
.code
_fmemcpy.3:
     __stw  <_ax
       ora  <_al
       beq  .done
     __ldw  <_di
     __stw  <_bx

       lda  <__fbank
       tam  #3
       lda  <__fptr+1
       and  #$1F
       ora  #$60
       sta  <__fptr+1

.copyloop:
       lda  [__fptr]
       sta  [_di]
       incw <__fptr
       incw <_di
       decw <_ax
       tstw <_ax
       bne  .copyloop
     __ldw  <_bx
.done:
       rts
#endasm
