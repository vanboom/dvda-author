#ifndef WINPORT_H_INCLUDED
#define WINPORT_H_INCLUDED

#if HAVE_CONFIG_H && !defined __CB__
#include "config.h"
#endif
#include "c_utils.h"
#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#if defined(__WIN32__) || defined(_WIN64) || defined(_WIN32) || defined(__WIN32) || defined (__WIN64)
#include <tchar.h>
#include <windows.h>
#include <sys/stat.h>
#include <string.h>

#ifndef __MINGW32__
#include <sys/resource.h>
#endif
#else
#include <sys/stat.h>
#endif


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

 inline static uint64_t stat_file_size(const char* filename)
{

    off_t file_size;

    int fd;
    FILE* fp;
    struct stat st;

    fd = open(filename, O_RDONLY);
    if (fd == -1)
    {
        close(fd);
        fprintf(stderr, "[ERR]" "   Impossible to open file %s for checking size.\n", filename);
        perror("       ");
        return 0;
    }

    fp = fdopen(fd, "r");

    if (fp == NULL)
    {
        close(fd);
        fprintf(stderr, "[ERR]" "   Impossible to fdopen file %s for checking size.\n", filename);
        perror("       ");
        return 0;
    }

    /* Ensure that the file is a regular file */

    if ((fstat(fd, &st) != 0) || (!S_ISREG(st.st_mode)))
    {
        close(fd);
        fprintf(stderr, "[ERR]" "   Impossible to fstat file %s for checking size.\n", filename);
        perror("       ");
        return 0;

    }

    if (fseeko(fp, 0 , SEEK_END) != 0)
    {
        close(fd);
        fprintf(stderr, "[ERR]" "   Impossible to fseeko file %s for checking size.\n", filename);
        perror("       ");
        return 0;
    }

    file_size = ftello(fp);

    fclose(fp);

    if (file_size == -1)
    {
      perror(ANSI_COLOR_RED "[ERR]");
      return 0;
    }

    return (uint64_t) file_size;
}

uint64_t read_file_size(FILE * fp, const char* filename);
int truncate_from_end(char* filename, uint64_t offset);
#endif


#ifndef S_OPEN
#  define S_OPEN(X, Y) {  if (file_exists(X.filename) && ! X.isopen) \
                           {\
                              if (! X.filesize) X.filesize =  stat_file_size(X.filename); \
                              X.fp = fopen(X.filename, Y); \
                              X.isopen = (X.fp != NULL); } }

#  define S_CLOSE(X) { if (X.isopen && X.fp != NULL) \
                           { \
                            fclose(X.fp); \
                            X.isopen = false; \
                            X.fp = NULL; \
                         } }

#endif // WINPORT_H_INCLUDED
