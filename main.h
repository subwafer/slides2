#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>

typedef struct {
    char *file_path;
    size_t content_size;
    char *file_contents;
    int slides_count;
    char **slides_content;
} Slideshow;

void read_entire_file(Slideshow *sh);

#endif // MAIN_H_
