#include <stdlib.h>
#ifndef USE_COUNT
#include <math.h>
#endif
#include "global.h"

#ifdef USE_COUNT
#if COUNT_RASTER_TYPE == RASTER_MAP_TYPE_UINT32
#define CELL_TYPE uint32
#define FLEN_TYPE unsigned int
#else
#define CELL_TYPE int32
#define FLEN_TYPE int
#endif
#define ORTHO_FLEN 1
#define DIA_FLEN 1
#else
#if LENGTH_RASTER_TYPE == RASTER_MAP_TYPE_FLOAT64
#define CELL_TYPE float64
#define FLEN_TYPE double
#else
#define CELL_TYPE float32
#define FLEN_TYPE float
#endif
#define ORTHO_FLEN ortho_flen
#define DIA_FLEN dia_flen
#endif

#define INDEX(row, col) (size_t)(row) * ncols + (col)
#define DIR_NULL (unsigned char)dir_map->null_value

#ifdef USE_LEAST_MEMORY
#ifdef USE_COUNT
#define FLEN_UPSTREAM fcnt_upstream_leastmem
#define UP(row, col) ((DIR(row, col) & 0x0000ff00) >> 8)
#define SET_UP(row, col) do { DIR(row, col) |= 0x80000000 | \
        FIND_UP(row, col) << 8; } while(0)
#define GET_DIR(row, col) (DIR(row, col) & 0x000000ff)
#define IS_NOTDONE(row, col) (DIR(row, col) & 0x80000000)
#else
#define FLEN_UPSTREAM flen_upstream_leastmem
#define UP(row, col) (unsigned char)(((int)DIR(row, col) - DIR(row, col)) * 256)
#define SET_UP(row, col) do { \
        DIR(row, col) = -DIR(row, col) - FIND_UP(row, col); } while(0)
#define GET_DIR(row, col) (unsigned char)abs(DIR(row, col))
#define IS_NOTDONE(row, col) (DIR(row, col) < 0)
#define FIND_UP(row, col) ( \
        (row > 0 ? \
         (col > 0 && GET_DIR(row - 1, col - 1) == SE ? NW : 0) + \
         (GET_DIR(row - 1, col) == S ? N : 0) + \
         (col < ncols - 1 && GET_DIR(row - 1, col + 1) == SW ? NE : 0) : 0) + \
        (col > 0 && GET_DIR(row, col - 1) == E ? W : 0) + \
        (col < ncols - 1 && GET_DIR(row, col + 1) == W ? E : 0) + \
        (row < nrows - 1 ? \
         (col > 0 && GET_DIR(row + 1, col - 1) == NE ? SW : 0) + \
         (GET_DIR(row + 1, col) == N ? S : 0) + \
         (col < ncols - 1 && GET_DIR(row + 1, col + 1) == NW ? SE : 0) : 0)) \
	 / 256.
#endif
#define DIR(row, col) dir_map->cells.CELL_TYPE[INDEX(row, col)]
#define IS_DONE(row, col) !IS_NOTDONE(row, col)
#define FLEN(row, col) DIR(row, col)
#define GET_FLEN(row, col) (IS_NOTDONE(row, col) ? 0 : FLEN(row, col))
#else
#ifdef USE_LESS_MEMORY
#ifdef USE_COUNT
#define FLEN_UPSTREAM fcnt_upstream_lessmem
#else
#define FLEN_UPSTREAM flen_upstream_lessmem
#endif
#define UP(row, col) FIND_UP(row, col)
#else
#ifdef USE_COUNT
#define FLEN_UPSTREAM fcnt_upstream
#else
#define FLEN_UPSTREAM flen_upstream
#endif
#define UP(row, col) up_cells[INDEX(row, col)]
static unsigned char *up_cells;
#endif
#define DIR(row, col) dir_map->cells.byte[INDEX(row, col)]
#define GET_DIR(row, col) DIR(row, col)
#define FLEN(row, col) flen_map->cells.CELL_TYPE[INDEX(row, col)]
#define GET_FLEN(row, col) FLEN(row, col)
#endif

#if defined USE_COUNT || !defined USE_LEAST_MEMORY
#define FIND_UP(row, col) ( \
        (row > 0 ? \
         (col > 0 && GET_DIR(row - 1, col - 1) == SE ? NW : 0) | \
         (GET_DIR(row - 1, col) == S ? N : 0) | \
         (col < ncols - 1 && GET_DIR(row - 1, col + 1) == SW ? NE : 0) : 0) | \
        (col > 0 && GET_DIR(row, col - 1) == E ? W : 0) | \
        (col < ncols - 1 && GET_DIR(row, col + 1) == W ? E : 0) | \
        (row < nrows - 1 ? \
         (col > 0 && GET_DIR(row + 1, col - 1) == NE ? SW : 0) | \
         (GET_DIR(row + 1, col) == N ? S : 0) | \
         (col < ncols - 1 && GET_DIR(row + 1, col + 1) == NW ? SE : 0) : 0))
#endif

static int nrows, ncols;

#ifndef USE_COUNT
static int ortho_dirs = N | S | W | E;
static FLEN_TYPE ortho_flen, dia_flen, half_ortho_flen, half_dia_flen;
#endif

static void trace_down(struct raster_map *
#ifndef USE_LEAST_MEMORY
                       , struct raster_map *
#endif
#ifndef USE_COUNT
                       , int
#endif
                       , int, int, FLEN_TYPE);
static FLEN_TYPE max_up(
#if defined USE_LESS_MEMORY || defined USE_LEAST_MEMORY
                           struct raster_map *,
#endif
#ifndef USE_LEAST_MEMORY
                           struct raster_map *,
#endif
                           int, int);

void FLEN_UPSTREAM(struct raster_map *dir_map
#ifndef USE_LEAST_MEMORY
                   , struct raster_map *flen_map
#endif
                   , int from_one)
{
    int row, col;

    nrows = dir_map->nrows;
    ncols = dir_map->ncols;

#ifndef USE_COUNT
    ortho_flen = (dir_map->dx + dir_map->dy) / 2;
    dia_flen = sqrt(pow(dir_map->dx, 2) + pow(dir_map->dy, 2));
    half_ortho_flen = ortho_flen / 2;
    half_dia_flen = dia_flen / 2;
#endif

#ifdef USE_LEAST_MEMORY
#pragma omp parallel for schedule(dynamic) private(col)
    for (row = 0; row < nrows; row++) {
        for (col = 0; col < ncols; col++)
            if (DIR(row, col) == DIR_NULL)
                FLEN(row, col) = 0;
            else
                SET_UP(row, col);
    }
    dir_map->null_value = 0;
#elifndef USE_LESS_MEMORY
    up_cells = calloc((size_t)nrows * ncols, sizeof *up_cells);
#pragma omp parallel for schedule(dynamic) private(col)
    for (row = 0; row < nrows; row++) {
        for (col = 0; col < ncols; col++)
            if (GET_DIR(row, col) != DIR_NULL)
                UP(row, col) = FIND_UP(row, col);
    }
#endif

#pragma omp parallel for schedule(dynamic) private(col)
    for (row = 0; row < nrows; row++) {
        for (col = 0; col < ncols; col++) {
#if !defined USE_COUNT || !defined USE_LEAST_MEMORY
            unsigned char dir = GET_DIR(row, col);
#endif

            /* if the current cell is not null and has no upstream cells, start
             * tracing down */
            if (
#ifdef USE_LEAST_MEMORY
                   IS_NOTDONE(row, col)
#else
                   dir != DIR_NULL
#endif
                   && !UP(row, col))
                trace_down(dir_map
#ifndef USE_LEAST_MEMORY
                           , flen_map
#endif
#ifndef USE_COUNT
                           , from_one
#endif
                           , row, col
#ifdef USE_COUNT
                           , 1
#else
                           , from_one ? (dir & ortho_dirs ? half_ortho_flen :
                                         half_dia_flen) : 1
#endif
                    );
        }
    }

#if !defined USE_LESS_MEMORY && !defined USE_LEAST_MEMORY
    free(up_cells);
#endif

    if (!from_one) {
        /* cells >= 1 are visited and cells == 0 are not; subtract 1 from them
         * to make visited cells start from 0 and unvisited cells null (-1); we
         * need to use this technique because the flen_map is initialized to 0
         * and using 0 as a valid cell value is not possible */
#ifdef USE_LEAST_MEMORY
        dir_map->null_value = -1;
#else
        flen_map->null_value--;
#endif
#pragma omp parallel for schedule(dynamic) private(col)
        for (row = 0; row < nrows; row++) {
            for (col = 0; col < ncols; col++)
                FLEN(row, col)--;
        }
    }
    /* else cells > 0 are visited and their values are desired; cells == 0 will
     * become null */
}

static void trace_down(struct raster_map *dir_map
#ifndef USE_LEAST_MEMORY
                       , struct raster_map *flen_map
#endif
#ifndef USE_COUNT
                       , int from_one
#endif
                       , int row, int col, FLEN_TYPE flen)
{
    int r = row, c = col;
    unsigned char dir = GET_DIR(row, col);
    FLEN_TYPE flen_up;

    /* find the downstream cell */
    switch (dir) {
    case NW:
        row--;
        col--;
        break;
    case N:
        row--;
        break;
    case NE:
        row--;
        col++;
        break;
    case W:
        col--;
        break;
    case E:
        col++;
        break;
    case SW:
        row++;
        col--;
        break;
    case S:
        row++;
        break;
    case SE:
        row++;
        col++;
        break;
    }

    /* accumulate the current cell itself */
    FLEN(r, c) = flen;

    /* if the downstream cell is null or any upstream cells of the downstream
     * cell have never been visited, stop tracing down */
    if (row < 0 || row >= nrows || col < 0 || col >= ncols ||
        GET_DIR(row, col) == DIR_NULL
#ifdef USE_COUNT
        || !(flen_up = max_up(
#if defined USE_LESS_MEMORY || defined USE_LEAST_MEMORY
                                 dir_map,
#endif
#ifndef USE_LEAST_MEMORY
                                 flen_map,
#endif
                                 row, col))
#endif
        ) {
#ifndef USE_COUNT
        if (from_one)
            FLEN(r, c) += dir & ortho_dirs ? half_ortho_flen : half_dia_flen;
#endif
        return;
    }

#ifdef USE_LEAST_MEMORY
    if (IS_DONE(row, col))
        return;
#endif

#ifndef USE_COUNT
    if (!(flen_up = max_up(
#if defined USE_LESS_MEMORY || defined USE_LEAST_MEMORY
                              dir_map,
#endif
#ifndef USE_LEAST_MEMORY
                              flen_map,
#endif
                              row, col)))
        return;
#endif

    /* use gcc -O2 or -O3 flags for tail-call optimization
     * (-foptimize-sibling-calls) */
    trace_down(dir_map
#ifndef USE_LEAST_MEMORY
               , flen_map
#endif
#ifndef USE_COUNT
               , from_one
#endif
               , row, col, flen_up);
}

/* if any upstream cells have never been visited, 0 is returned; otherwise, the
 * sum of upstream accumulation is returned */
static FLEN_TYPE max_up(
#if defined USE_LESS_MEMORY || defined USE_LEAST_MEMORY
                           struct raster_map *dir_map,
#endif
#ifndef USE_LEAST_MEMORY
                           struct raster_map *flen_map,
#endif
                           int row, int col)
{
    unsigned char up = UP(row, col);
    FLEN_TYPE max = 0, flen;

#ifdef USE_LEAST_MEMORY
#pragma omp flush(dir_map)
#else
#pragma omp flush(flen_map)
#endif
    if (up & NW) {
        if (!(flen = GET_FLEN(row - 1, col - 1)))
            return 0;
        if (flen + DIA_FLEN > max)
            max = flen + DIA_FLEN;
    }
    if (up & N) {
        if (!(flen = GET_FLEN(row - 1, col)))
            return 0;
        if (flen + ORTHO_FLEN > max)
            max = flen + ORTHO_FLEN;
    }
    if (up & NE) {
        if (!(flen = GET_FLEN(row - 1, col + 1)))
            return 0;
        if (flen + DIA_FLEN > max)
            max = flen + DIA_FLEN;
    }
    if (up & W) {
        if (!(flen = GET_FLEN(row, col - 1)))
            return 0;
        if (flen + ORTHO_FLEN > max)
            max = flen + ORTHO_FLEN;
    }
    if (up & E) {
        if (!(flen = GET_FLEN(row, col + 1)))
            return 0;
        if (flen + ORTHO_FLEN > max)
            max = flen + ORTHO_FLEN;
    }
    if (up & SW) {
        if (!(flen = GET_FLEN(row + 1, col - 1)))
            return 0;
        if (flen + DIA_FLEN > max)
            max = flen + DIA_FLEN;
    }
    if (up & S) {
        if (!(flen = GET_FLEN(row + 1, col)))
            return 0;
        if (flen + ORTHO_FLEN > max)
            max = flen + ORTHO_FLEN;
    }
    if (up & SE) {
        if (!(flen = GET_FLEN(row + 1, col + 1)))
            return 0;
        if (flen + DIA_FLEN > max)
            max = flen + DIA_FLEN;
    }

    return max;
}
