#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <stdint.h>

#ifdef _MSC_VER
#include <winsock2.h>
/* gettimeofday.c */
int gettimeofday(struct timeval *, struct timezone *);
#else
#include <sys/time.h>
#endif
#include "raster.h"

#define REALLOC_INCREMENT 1024

#define NE 128
#define N 64
#define NW 32
#define W 16
#define SW 8
#define S 4
#define SE 2
#define E 1

#ifdef USE_FLOAT64_LENGTH
#define LENGTH_RASTER_TYPE RASTER_TYPE_FLOAT64
#else
#define LENGTH_RASTER_TYPE RASTER_TYPE_FLOAT32
#endif

/* timeval_diff.c */
long long timeval_diff(struct timeval *, struct timeval *, struct timeval *);

/* recode.c */
double recode_encoding(double, void *);
double recode_degree(double, void *);

/* uflen.c */
void uflen(struct raster_map *, struct raster_map *, int, int);

/* uflen_leastmem.c */
void uflen_leastmem(struct raster_map *, int);

/* uflen_lessmem.c */
void uflen_lessmem(struct raster_map *, struct raster_map *, int);

/* uflen_moremem.c */
void uflen_moremem(struct raster_map *, struct raster_map *, int);

#endif
