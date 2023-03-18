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

    return 0;
}

void read_entire_file(Slideshow *sh) {
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

    if (fseek(file, 0, SEEK_SET) > 0) {
        fprintf(stderr, "ERROR: Could not seek to start of file: %s | -> %s\n", sh->file_path, strerror(errno));
        goto cleanup;
    }

    sh->file_contents = malloc(fc * sizeof(char));

    size_t rc = fread(sh->file_contents, 1, fc, file);
    assert(rc == (size_t) fc);

    fwrite(sh->file_contents, 1, fc, stdout);

cleanup:
    if (file) fclose(file);
    if (sh->file_contents) free(sh->file_contents);
}
