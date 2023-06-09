#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>

typedef struct {
    char *file_path;
    size_t content_size;
    char *file_contents;
    int slide_count;
    char **slides_content;
} Slideshow;

void split_slides(Slideshow *sh);
int read_entire_file(Slideshow *sh);
void stdout_display(const Slideshow *sh);

#endif // MAIN_H_
