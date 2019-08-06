
#ifndef _VTERM_PRIVATE_H_
#define _VTERM_PRIVATE_H_

#include <unistd.h>
#include <termios.h>

#include <sys/types.h>

#include "mouse_driver.h"
#include "color_cache.h"

#ifndef NOCURSES

#ifdef __linux__
#include <ncursesw/curses.h>
#endif

#ifdef __FREEBSD__
#include <ncurses/cursesw.h>
#endif

#endif

#define ESEQ_BUF_SIZE           128             // escape buffer max
#define UTF8_BUF_SIZE           5               // 4 bytes + 0-terminator

#define STATE_ALT_CHARSET       (1 << 1)
#define STATE_ESCAPE_MODE       (1 << 2)
#define STATE_UTF8_MODE         (1 << 3)

#define STATE_PIPE_ERR          (1 << 8)
#define STATE_CHILD_EXITED      (1 << 9)

#define STATE_CURSOR_INVIS      (1 << 10)
#define STATE_SCROLL_SHORT      (1 << 11)       /*
                                                    scroll region is not
                                                    full height
                                                */
#define STATE_REPLACE_MODE      (1 << 12)
#define STATE_NO_WRAP           (1 << 13)
#define STATE_AUTOMATIC_LF      (1 << 14)       //  DEC SM "20"

#define IS_MODE_ESCAPED(x)      (x->internal_state & STATE_ESCAPE_MODE)
#define IS_MODE_ACS(x)          (x->internal_state & STATE_ALT_CHARSET)
#define IS_MODE_UTF8(x)         (x->internal_state & STATE_UTF8_MODE)

#define VTERM_TERM_MASK         0xFF            /*
                                                    the lower 8 bits of the
                                                    flags value specify term
                                                    type.
                                                */

#define VTERM_MOUSE_X10         (1 << 1)
#define VTERM_MOUSE_VT200       (1 << 2)
#define VTERM_MOUSE_SGR         (1 << 7)
#define VTERM_MOUSE_HIGHLIGHT   (1 << 12)
#define VTERM_MOUSE_ALTSCROLL   (1 << 13)


typedef struct _vterm_cmap_s   vterm_cmap_t;

struct _vterm_cmap_s
{
    short           private_color;              /*
                                                    the color that the guest
                                                    application is expecting
                                                    to use.
                                                */

    short           global_color;               /*
                                                    the internal color number
                                                    we have mapped the
                                                    private_color to.
                                                */

    float           red;                        //  RGB values of the color
    float           green;
    float           blue;

    vterm_cmap_t    *next;                      //  next entry in the map
    vterm_cmap_t    *prev;                      //  prev entry in the map
};


struct _vterm_desc_s
{
    int             rows, cols;                 // terminal height & width
    vterm_cell_t    **cells;
    vterm_cell_t    last_cell;                  // contents of last cell write

    unsigned int    buffer_state;               // internal state control

    attr_t          curattr;                    // current attribute set
    int             colors;                     // current color pair

    int             default_colors;             // default fg/bg color pair

    int             crow, ccol;                 // current cursor column & row
    int             scroll_min;                 // top of scrolling region
    int             scroll_max;                 // bottom of scrolling region
    int             saved_x, saved_y;           // saved cursor coords

    int             fg;                         // current fg color
    int             bg;                         // current bg color
    short           f_rgb[3];                   // current fg RGB values
    short           b_rgb[3];                   // current bg RGB values
};

typedef struct _vterm_desc_s    vterm_desc_t;

struct _vterm_s
{
    vterm_desc_t    vterm_desc[2];              // normal buffer and alt buffer
    int             vterm_desc_idx;             // index of active buffer;

#ifndef NOCURSES
    WINDOW          *window;                    // curses window
    vterm_cmap_t    *cmap_head;
#endif
    char            ttyname[96];                // populated with ttyname_r()

    char            title[128];                 /*
                                                    possibly the name of the
                                                    application running in the
                                                    terminal.  the data is
                                                    supplied by the Xterm OSC
                                                    code sequences.
                                                */


    char            esbuf[ESEQ_BUF_SIZE];       /*
                                                    0-terminated string. Does
                                                    NOT include the initial
                                                    escape (\x1B) character.
                                                */

    int             esbuf_len;                  /*
                                                    length of buffer. The
                                                    following property is
                                                    always kept:
                                                    esbuf[esbuf_len] == '\0'
                                                */

    char            utf8_buf[UTF8_BUF_SIZE];    /*
                                                    0-terminated string that
                                                    is the UTF-8 coding.
                                                */

    char            reply_buf[32];              /*
                                                    some CSI sequences
                                                    expect a reply.  here's
                                                    where they go.
                                                */

    ssize_t         reply_buf_sz;               //  size of reply

    int             utf8_buf_len;               //  number of utf8 bytes

    int             pty_fd;                     /*
                                                    file descriptor for the pty
                                                    attached to this terminal.
                                                */

    pid_t           child_pid;                  //  pid of the child process
    uint32_t        flags;                      //  user options
    unsigned int    internal_state;             //  internal state control

    uint16_t        mouse;                      //  mouse mode
    mouse_config_t  *mouse_config;              /*
                                                    saves and restores the
                                                    state of the mouse
                                                    if we are sharing it with
                                                    others.
                                                */

    char            **keymap_str;               //  points to keymap key
    uint32_t        *keymap_val;                //  points to keymap ke value
    int             keymap_size;                //  size of the keymap

    struct termios  term_state;                 /*
                                                    stores data returned from
                                                    tcgetattr()
                                                */

    char            *exec_path;                 //  optional binary path to use
    char            **exec_argv;                //  instead of starting shell.

    char            *debug_filepath;
    int             debug_fd;

    uint32_t        event_mask;                 /*
                                                    specificies which events
                                                    should be passed on to
                                                    the custom event hook.
                                                */

    // internal callbacks
    int             (*write)            (vterm_t *, uint32_t);
    int             (*esc_handler)      (vterm_t *);
    int             (*rs1_reset)        (vterm_t *, char *);
    ssize_t         (*mouse_driver)     (vterm_t *, unsigned char *);

    // callbacks that the implementer can specify
    void            (*event_hook)       (vterm_t *, int, void *);
    short           (*pair_select)      (vterm_t *, short, short);
    int             (*pair_split)       (vterm_t *, short, short *, short *);
};

#define VTERM_CELL(vterm_ptr, x, y)     ((y * vterm_ptr->cols) + x)

#endif
