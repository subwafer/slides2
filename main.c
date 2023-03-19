#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>

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

bool DEBUG_MODE = false;

void handle_cli_args(int argc, char **argv) {

    if (argc > 0) {
        for (int i = 0; i < argc; i++) {
            if (strcmp(argv[i], "--debug") == 0) {
                DEBUG_MODE = true;
                printf("------DEBUG MODE ACTIVE-------");
            }
        }
    }

}

int main(int argc, char **argv) {
    handle_cli_args(argc, argv);

    Slideshow sh;

    if (DEBUG_MODE) {
        sh.file_path = "demos/show1.md";
    }


    read_entire_file(&sh);

    split_slides(&sh);


    for (int i = 0; i < sh.slide_count; i++) {
        free(sh.slides_content[i]);
    }

    free(sh.slides_content);

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
    int i = 0;

    // NOTE: I'm still not 100% certain on when to use calloc vs malloc.
    // Calloc initializes every byte to zero. While malloc doesn't.
    sh->slides_content = calloc(slide_count, sizeof(char*));
    sh->slides_content[slide_count - 1] = calloc(sh->content_size, sizeof(char)); // NOTE: using content size is probably a bad idea?

    for (const char *c = sh->file_contents; *c; c++) {
        switch (state) {
            case READY:
                switch (*c) {
                    case '-':
                        state = START_DELIM;
                        if (*(c + 1) != '-') { // this handles the bullet points
                            sh->slides_content[slide_count - 1][i] = *c;
                        }
                        break;
                    default:
                        sh->slides_content[slide_count - 1][i] = *c;
                        break;
                }
                break;
            case START_DELIM:
                switch (*c) {
                    case '-':
                        state = FOUND_DELIM;
                        break;
                    default:
                        state = READY;
                        break;
                }
                break;
            case FOUND_DELIM:
                switch (*c) {
                    case '\n':
                        slide_count++;
                        state = READY;

                        // HERE WE REALLOC FOR THE NEXT SLIDE

                        sh->slides_content = realloc(sh->slides_content, slide_count * sizeof(char*));
                        sh->slides_content[slide_count - 1] = calloc(sh->content_size, sizeof(char)); // NOTE: using content size is probably a bad idea?
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
        i++;
    }

    printf("SLIDE COUNT: %d\n", slide_count);

    sh->slide_count = slide_count;




    // TEMP DISPLAY OUT

    printf("FINAL ------------\n");
    for (int sc = 0; sc < sh->slide_count; sc++) {
        for (size_t i = 0; i < sh->content_size; i++) {
            printf("%c", sh->slides_content[sc][i]);
        }
        getchar();
    }
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
