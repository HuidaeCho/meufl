ifeq ($(OS),Windows_NT)
	GDAL_CFLAGS=-I/c/OSGeo4W/include
	GDAL_LIBS=/c/OSGeo4W/lib/gdal_i.lib
	EXT=.exe
else
	GDAL_LIBS=`gdal-config --libs`
endif
CFLAGS=-Wall -Werror -O3 -fopenmp $(GDAL_CFLAGS)
LDFLAGS=-O3 -fopenmp -lm

all: meufl$(EXT)

clean:
	$(RM) *.o

meufl$(EXT): \
	main.o \
	timeval_diff.o \
	raster.o \
	recode.o \
	flen.o \
	flen_upstream_moremem.o \
	flen_upstream_lessmem.o \
	flen_upstream_leastmem.o
	$(CC) $(LDFLAGS) -o $@ $^ $(GDAL_LIBS)

*.o: global.h raster.h
f*_upstream*.o: flen_upstream_funcs.h
