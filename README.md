Based on ROTE, libvterm is a terminal emulator library which attempts to mimic
VT100, rxvt, xterm and xterm 256 color capabilities. Although the natural display
apparatus is curses, current contributions from Hitachi-ID have allowed it to use
a stream buffer for output.

The build system for libvterm uses CMake and it automatically detects a variety of
different settings and libraries.  A default build tries to include support for
curses/ncurses.  If you want to suppress the behavior, specify the following:

cmake -DDEFINE_CURSES=OFF
