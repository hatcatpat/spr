# spr
## a terminal pixel editor for .chr files

<p align="center">
  <img alt="spr example with a familiar fellow" src="https://user-images.githubusercontent.com/39860407/226191884-4d1e8bf3-befc-4d58-a064-b9365ad080bc.png"/>
</p>


i was tinkering around with the nes and 6502 assembly, but i needed a way to:

a) edit chr files

b) understand chr files

if you want a far more ***beautiful*** and ***practical*** option, i recommend [nasu](https://100r.co/site/nasu.html)

## chr format

the chr files used here contain 16x16 sprites, each 8x8 pixels in size, with a possible 4 colors.

each byte of a chr file describes an 8-pixel wide row, with each bit corresponding to one of those pixels. the tricky part, is that you require **two** of these bytes to fully describe the color of a pixel. these two bytes are separated by 8 bytes.

* a bit in the left byte (as we will call it): color = 1
* a bit in the right byte: color = 2
* and if both bits are enabled: color = 3
* if none of the bits are enabled: color = 0

so each row takes up 2 bytes, and there are 8 rows per sprite, giving a total of 16 bytes per sprite

**example:**
```
the row:    1030 2111
left byte:  1010 0111 = $a7
right byte: 0010 1000 = $28

data[0] = $a7
data[8] = $28
```

## building

* requirements:
  * ncurses
  * c compiler (gcc/tcc)
  * linux? maybe?

* makefile has all your favourites:
  * make run
  * make clean
  * make debug

## config

follows the [suckless](suckless.org) style of config, so just edit the macros in config.h

## usage

./spr filename

mouse: moves cursor
arrows: moves cursor
wasd: jumps to next sprite
q: quit
r: reload file
enter: saves file
%: toggles wrap. when enabled, causes the cursor to wrap around the current sprite rather than changing to a different sprite

status bar (in order of appearance):
* file name (- for unedited, + for edited)
* current sprite
* cursor position
* current pixel
* wrap (- for disabled, + for enabled)
* current row (in chr format, showing left byte + right byte)
