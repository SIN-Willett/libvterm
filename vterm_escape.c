#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "vterm.h"
#include "vterm_private.h"
#include "vterm_escape.h"
#include "vterm_escape_suffixes.h"
#include "vterm_csi.h"
#include "vterm_osc.h"
#include "vterm_osc_DA.h"
#include "vterm_render.h"
#include "vterm_misc.h"
#include "vterm_buffer.h"
#include "macros.h"


static int
vterm_interpret_esc_xterm_dsc(vterm_t *vterm);

static int
vterm_interpret_esc_scs(vterm_t *vterm);


void
vterm_escape_start(vterm_t *vterm)
{
    vterm->internal_state |= STATE_ESCAPE_MODE;

    // zero out the escape buffer just in case
    vterm->esbuf_len = 0;
    vterm->esbuf[0] = '\0';

    vterm->esc_handler = NULL;

    return;
}

void
vterm_escape_cancel(vterm_t *vterm)
{
    vterm->internal_state &= ~STATE_ESCAPE_MODE;

    // zero out the escape buffer for the next run
    vterm->esbuf_len = 0;
    vterm->esbuf[0] = '\0';

    vterm->esc_handler = NULL;

    return;
}

void
vterm_interpret_escapes(vterm_t *vterm)
{
    char                firstchar;
    char                *lastchar;

    static void         *interim_table[128] =
                            {
                                [0] = &&interim_char_none,
                                [']'] = &&interim_rbracket,
                                ['['] = &&interim_lbracket,
                                [')'] = &&interim_rparth,
                                ['('] = &&interim_lparth,
                                ['P'] = &&interim_char_P,
                            };

    int                 retval = 0;


    firstchar = vterm->esbuf[0];
    lastchar = &vterm->esbuf[vterm->esbuf_len - 1];

    // too early to do anything
    if(!firstchar) return;

    retval = vterm_interpret_escapes_simple(vterm);
    if(retval > 0) return;

    SWITCH(interim_table, (unsigned int)firstchar, 0);

    // looks like an complete xterm Operating System Command
    interim_rbracket:
        // term type linux sends this to reset FG and BG colors to default
        if(*lastchar == 'R')
        {
            vterm_escape_cancel(vterm);
            goto interim_run;
        }

        if(validate_xterm_escape_suffix(lastchar))
        {
            vterm->esc_handler = vterm_interpret_xterm_osc;
        }
        goto interim_run;

    // we have a complete csi escape sequence: interpret it
    interim_lbracket:
        if(validate_csi_escape_suffix(lastchar))
        {
            vterm->esc_handler = vterm_interpret_csi;
        }
        goto interim_run;

    // SCS G0 sequence - discards for now
    interim_lparth:
        if(validate_scs_escape_suffix(lastchar))
        {
            vterm->esc_handler = vterm_interpret_esc_scs;
        }
        goto interim_run;

    // SCS G1 sequence - discards for now
    interim_rparth:
        if(validate_scs_escape_suffix(lastchar))
        {
            vterm->esc_handler = vterm_interpret_esc_scs;
        }
        goto interim_run;


    // DCS sequence - starts in P and ends in Esc backslash
    interim_char_P:
        if( vterm->esbuf_len > 2
            && vterm->esbuf[vterm->esbuf_len - 2] == '\x1B'
            && *lastchar == '\\' )
        {
            vterm->esc_handler = vterm_interpret_esc_xterm_dsc;
        }
        goto interim_run;

    interim_char_none:
        vterm_escape_cancel(vterm);
        return;

    interim_run:

    // if an escape handler has been configured, run it.
    if(vterm->esc_handler != NULL)
    {
        vterm->esc_handler(vterm);
        vterm_escape_cancel(vterm);

        return;
    }

    return;
}

int
vterm_interpret_esc_xterm_dsc(vterm_t *vterm)
{
    /*
        TODO

        accept DSC (ESC-P) sequences from xterm but don't do anything
        with them - just toss them to /dev/null for now.
    */

    const char  *p;

    p = vterm->esbuf + 1;

    if( *p=='+'
        && *(p+1)=='q'
        && isxdigit( *(p+2) )
        && isxdigit( *(p+3) )
        && isxdigit( *(p+4) )
        && isxdigit( *(p+5) ) )
        {
        /* we have a valid code, and we'll promptly ignore it */
        }

    return 0;
}

static int
vterm_interpret_esc_scs(vterm_t *vterm)
{
    const char  *p;

    p = vterm->esbuf;

    // not the most elegant way to handle these.  todo: improve later.
    if(*p == '(' && p[1] == '0')
    {
        vterm->internal_state |= STATE_ALT_CHARSET;
    }

    if(*p == '(' && p[1] == 'B')
    {
        vterm->internal_state &= ~STATE_ALT_CHARSET;
    }

    // G1 sequence - unused
    if(*p == ')') {}

    p++;
    // could do something with the codes

    // return the number of bytes handled
    return 2;
}


