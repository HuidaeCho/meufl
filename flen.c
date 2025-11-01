#include "global.h"

void flen(struct raster_map *dir_map, struct raster_map *flen_map,
          int use_lessmem, int from_one)
{
    switch (use_lessmem) {
    case 0:
	flen_upstream(dir_map, flen_map, from_one);
        break;
    case 1:
        flen_upstream_lessmem(dir_map, flen_map, from_one);
        break;
    case 2:
        flen_upstream_leastmem(dir_map, from_one);
        break;
    }
}
