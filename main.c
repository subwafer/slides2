#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "main.h"

/*
**
** Creates slideshows from a file of some sort (.md for now)
** Displays slideshows in native gui (SDL2)
**
** 1. read entire file (as bytes) into continuous buffer
** 2. get the number of slides (delimited by "---")
** 3. display each slide one at a time in stdout
** 4. add user control to switch slides and jump to specific slide
** 5. figure out SDL2
**
*/

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;

    Slideshow sh;

    sh.file_path = "demos/show1.md";

    read_entire_file(&sh);

    split_slides(&sh);

    return 0;
}

void split_slides(Slideshow *sh) {
    // I think I am going to use a state machine again.
    // it worked well and was cleaner than if elses

    enum states {
        READY,
        START_DELIM,
        FOUND_DELIM,
    };

    int state = 0;
    int slide_count = 1;

    for (size_t i = 0; i < sh->content_size; i++) {
        char c = sh->file_contents[i];

        // TODO: After getting each slide, alloc and realloc mem for char **slides_content;
        // look at the todocurses project for implementation example
        switch (state) {
            case READY:
                switch (c) {
                    case '-':
                        printf("%c", c);
                        state = START_DELIM;
                        break;
                    default:
                        printf("%c", c);
                        break;
                }
                break;
            case START_DELIM:
                switch (c) {
                    case '-':
                        printf("%c", c);
                        state = FOUND_DELIM;
                        break;
                    default:
                        state = READY;
                        break;
                }
                break;
            case FOUND_DELIM:
                switch (c) {
                    case '\n':
                        printf("%c", c);
                        slide_count++;
                        getchar();
                        state = READY;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }

    printf("SLIDE COUNT: %d\n", slide_count);
}

void read_entire_file(Slideshow *sh) {
    sh->file_contents = NULL;

    FILE *file = fopen(sh->file_path, "r");
    if (file == NULL) {
        // TODO: Create a "logger" of some sort
        fprintf(stderr, "ERROR: Could not open file: %s | -> %s\n", sh->file_path, strerror(errno));
        goto cleanup;
    }

    // seek to end of file

    if (fseek(file, 0, SEEK_END) > 0) {
        fprintf(stderr, "ERROR: Could not seek to end of file: %s | -> %s\n", sh->file_path, strerror(errno));
        goto cleanup;
    }

    long fc = ftell(file);
    if (fc < 0) {
        fprintf(stderr, "ERROR: Failed reading cursor for file: %s | -> %s\n", sh->file_path, strerror(errno));
        goto cleanup;
    }

    sh->file_contents = malloc(fc * sizeof(char));

    if (fseek(file, 0, SEEK_SET) > 0) {
        fprintf(stderr, "ERROR: Could not seek to start of file: %s | -> %s\n", sh->file_path, strerror(errno));
        goto cleanup;
    }

    size_t rc = fread(sh->file_contents, 1, fc, file);
    assert(rc == (size_t) fc);

    sh->content_size = rc;


cleanup:
    if (file) fclose(file);
}
