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
	uflen.o \
	uflen_moremem.o \
	uflen_lessmem.o \
	uflen_leastmem.o
	$(CC) $(LDFLAGS) -o $@ $^ $(GDAL_LIBS)

*.o: global.h raster.h
uflen_*.o: uflen_funcs.h
