
#ifndef _VTERM_COLORS_H_
#define _VTERM_COLORS_H_

#include "vterm.h"

struct _rgb_values_s
{
    short   r;
    short   g;
    short   b;
};

struct _hsl_values_s
{
    float   h;
    float   s;
    float   l;
};

struct _cie_values_s
{
    float   l;
    float   a;
    float   b;
};

typedef struct _rgb_values_s    rgb_values_t;
typedef struct _hsl_values_s    hsl_values_t;
typedef struct _cie_values_s    cie_values_t;


struct _color_pair_s
{
    uint8_t                 ref;

    short                   num;
    short                   fg;
    short                   bg;

    rgb_values_t            rgb_values[2];
    hsl_values_t            hsl_values[2];
    cie_values_t            cie_values[2];

    struct _color_pair_s    *next;
    struct _color_pair_s    *prev;
};


typedef struct _color_pair_s    color_pair_t;

struct _color_cache_s
{
    short           pair_count;

    color_pair_t    *pair_head;
};

typedef struct _color_cache_s   color_cache_t;

color_cache_t*
color_cache_init(int pairs);

short
color_cache_add_new_pair(color_cache_t *color_cache, short fg, short bg);

void
color_cache_destroy(color_cache_t *color_cache);

short
color_cache_find_pair(color_cache_t *color_cache, short fg, short bg);

short
color_cache_find_exact_color(color_cache_t *color_cache, short color,
    short r, short g, short b);

short
color_cache_find_nearest_color(color_cache_t *color_cache,
    short r, short g, short b);

short
color_cache_split_pair(color_cache_t *color_cache, short pair_num,
    short *fg, short *bg);

#endif

