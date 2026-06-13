#define _MAIN_C_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gdal.h>
#include <omp.h>
#ifdef _MSC_VER
#include <winsock2.h>
#else
#include <sys/time.h>
#endif
#include "global.h"

int main(int argc, char *argv[])
{
    int i;
    int print_usage = 1, calc_md5 = 0, from_one = 0, use_lessmem =
        0, compress_output = 0;
    double (*recode)(double, void *) = NULL;
    int *recode_data = NULL, encoding[8];
    char *dir_path = NULL, *dir_opts = NULL, *flen_path = NULL;
    int num_threads = 0;
    struct raster_map *dir_map, *flen_map = NULL;
    struct timeval first_time, start_time, end_time;

    gettimeofday(&first_time, NULL);

    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            int j, n = strlen(argv[i]);
            int unknown = 0;

            for (j = 1; j < n && !unknown; j++) {
                switch (argv[i][j]) {
                case 'o':
                    if (i == argc - 1) {
                        fprintf(stderr, "-%c: Missing output filename\n",
                                argv[i][j]);
                        print_usage = 2;
                        break;
                    }
                    flen_path = argv[++i];
                    break;
                case 'm':
                    calc_md5 = 1;
                    break;
                case '1':
                    from_one = 1;
                    break;
                case 'l':
                    use_lessmem = 1;
                    break;
                case 'L':
                    use_lessmem = 2;
                    break;
                case 'z':
                    compress_output = 1;
                    break;
                case 'e':
                    if (i == argc - 1) {
                        fprintf(stderr, "-%c: Missing encoding\n",
                                argv[i][j]);
                        print_usage = 2;
                        break;
                    }
                    if (strcmp(argv[++i], "power2") == 0) {
                        recode = NULL;
                        recode_data = NULL;
                        break;
                    }
                    else if (strcmp(argv[i], "taudem") == 0) {
                        int k;

                        for (k = 1; k < 9; k++)
                            encoding[k % 8] = 9 - k;
                    }
                    else if (strcmp(argv[i], "45degree") == 0) {
                        int k;

                        for (k = 0; k < 8; k++)
                            encoding[k] = 8 - k;
                    }
                    else if (strcmp(argv[i], "degree") == 0) {
                        recode = recode_degree;
                        recode_data = NULL;
                        break;
                    }
                    else if (sscanf
                             (argv[i], "%d,%d,%d,%d,%d,%d,%d,%d",
                              &encoding[0], &encoding[1], &encoding[2],
                              &encoding[3], &encoding[4], &encoding[5],
                              &encoding[6], &encoding[7]) != 8) {
                        fprintf(stderr, "%s: Invalid encoding\n", argv[i]);
                        print_usage = 2;
                        break;
                    }
                    recode = recode_encoding;
                    recode_data = encoding;
                    break;
                case 'D':
                    if (i == argc - 1) {
                        fprintf(stderr,
                                "-%c: Missing GDAL options for input direction\n",
                                argv[i][j]);
                        print_usage = 2;
                        break;
                    }
                    dir_opts = argv[++i];
                    break;
                case 't':
                    if (i == argc - 1) {
                        fprintf(stderr, "-%c: Missing number of threads\n",
                                argv[i][j]);
                        print_usage = 2;
                        break;
                    }
                    num_threads = atoi(argv[++i]);
                    break;
                default:
                    unknown = 1;
                    j--;
                    break;
                }
            }
            if (unknown) {
                fprintf(stderr, "%c: Unknown flag\n", argv[i][j]);
                print_usage = 2;
                break;
            }
        }
        else if (!dir_path) {
            dir_path = argv[i];
            print_usage = 0;
        }
        else {
            fprintf(stderr, "%s: Unable to process extra arguments\n",
                    argv[i]);
            print_usage = 2;
            break;
        }
    }

    if (!flen_path && !calc_md5) {
        fprintf(stderr, "Either -o or -m is required\n");
        print_usage = 2;
    }

    if (print_usage) {
        if (print_usage == 2)
            printf("\n");
        printf("Usage: meufl OPTIONS dir\n\n"
               "  dir\t\tInput flow direction raster (e.g., gpkg:file.gpkg:layer)\n"
               "  -o uflen\tOutput upstream flow length GeoTIFF (or -m required)\n"
               "  -m\t\tCalculate MD5 (or -o required)\n"
               "  -1\t\tCount from 1 (default: 0)\n"
               "  -l\t\tUse less memory\n"
               "  -L\t\tUse least memory\n"
               "  -z\t\tCompress output GeoTIFF\n"
               "  -e encoding\tInput flow direction encoding\n"
               "\t\tpower2 (default): 2^0-7 (E-NE CW) (e.g., r.terraflow, ArcGIS)\n"
               "\t\ttaudem: 1-8 (E-SE CCW) (e.g., d8flowdir)\n"
               "\t\t45degree: 1-8 (NE-E CCW) (e.g., r.watershed)\n"
               "\t\tdegree: (0,360] (E-E CCW)\n"
               "\t\tE,SE,S,SW,W,NW,N,NE: custom (e.g., 1,8,7,6,5,4,3,2 for taudem)\n"
               "  -D opts\tComma-separated list of GDAL options for dir\n"
               "  -t threads\tNumber of threads (default: OMP_NUM_THREADS)\n");
        exit(print_usage == 1 ? EXIT_SUCCESS : EXIT_FAILURE);
    }

    if (num_threads == 0)
        num_threads = omp_get_max_threads();
    else {
        if (num_threads < 0) {
            num_threads += omp_get_num_procs();
            if (num_threads < 1)
                num_threads = 1;
        }
        omp_set_num_threads(num_threads);
    }

    printf("Using %d threads...\n", num_threads);

    GDALAllRegister();

    printf("Reading flow direction raster <%s>...\n", dir_path);
    gettimeofday(&start_time, NULL);
    if (!(dir_map = read_raster(dir_path, dir_opts, use_lessmem == 2 ?
                                LENGTH_RASTER_TYPE : RASTER_TYPE_BYTE, 0,
                                recode, recode_data))) {
        fprintf(stderr, "%s: Failed to read flow direction raster\n",
                dir_path);
        exit(EXIT_FAILURE);
    }
    gettimeofday(&end_time, NULL);
    printf("Input time for flow direction: %lld microsec\n",
           timeval_diff(NULL, &end_time, &start_time));

    if (use_lessmem != 2) {
        flen_map =
            init_raster(dir_map->nrows, dir_map->ncols, LENGTH_RASTER_TYPE);
        copy_raster_metadata(flen_map, dir_map);
    }

    printf("Calculating flow length...\n");
    gettimeofday(&start_time, NULL);
    uflen(dir_map, flen_map, use_lessmem, from_one);
    gettimeofday(&end_time, NULL);
    printf("Computation time for flow length: %lld microsec\n",
           timeval_diff(NULL, &end_time, &start_time));
    if (use_lessmem != 2)
        free_raster(dir_map);

    if (flen_path) {
        if (use_lessmem == 2)
            dir_map->compress = compress_output;
        else
            flen_map->compress = compress_output;

        printf("Writing flow length raster <%s>...\n", flen_path);
        gettimeofday(&start_time, NULL);
        if (write_raster
            (flen_path, use_lessmem == 2 ? dir_map : flen_map,
             RASTER_TYPE_AUTO) > 0) {
            printf("%s: Failed to write flow length raster\n", flen_path);
            free_raster(use_lessmem == 2 ? dir_map : flen_map);
            exit(EXIT_FAILURE);
        }
        gettimeofday(&end_time, NULL);
        printf("Output time for flow length: %lld microsec\n",
               timeval_diff(NULL, &end_time, &start_time));
    }

    if (calc_md5) {
        printf("Calculating flow length MD5 hash...\n");
        gettimeofday(&start_time, NULL);
        printf("MD5: ");
        print_md5(use_lessmem == 2 ? dir_map : flen_map);
        gettimeofday(&end_time, NULL);
        printf("MD5 hash time for flow length: %lld microsec\n",
               timeval_diff(NULL, &end_time, &start_time));
    }

    free_raster(use_lessmem == 2 ? dir_map : flen_map);

    gettimeofday(&end_time, NULL);
    printf("Total elapsed time: %lld microsec\n",
           timeval_diff(NULL, &end_time, &first_time));

    exit(EXIT_SUCCESS);
}
