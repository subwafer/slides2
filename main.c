#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

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

bool DEMO_MODE = false;
bool STDOUT_MODE = false;

void handle_cli_args(int argc, char **argv, Slideshow *sh) {

    if (argc > 0) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "--demo") == 0) {
                printf("------DEMO MODE ACTIVE-------\n");
                DEMO_MODE = true;
                sh->file_path = "./demos/show1.md";

            } else if (strcmp(argv[i], "--stdout") == 0) {
                STDOUT_MODE = true;
            } else {
                sh->file_path = NULL;

                int start_idx = 0;
                int end_idx = strlen(argv[i]);

                for (size_t j = 0; j < strlen(argv[i]); j++) {
                    if (argv[i][j] == '=') {
                        start_idx = j;
                    }
                }

                sh->file_path = malloc((end_idx - start_idx) * sizeof(char));

                int index = 0;

                for (size_t j = start_idx + 1; j < strlen(argv[i]); j++) {
                    sh->file_path[index] = argv[i][j];
                    index++;
                }
            }
        }
    }

}

void sdl_test(void) {

    int rendererFlags, windowFlags;

    rendererFlags = SDL_RENDERER_ACCELERATED;

    windowFlags = 0;

    int SCREEN_WIDTH = 800;
    int SCREEN_HEIGHT = 600;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Couldn't init SDL: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_Window *window = SDL_CreateWindow("Slides",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT,
                                          windowFlags);

    if (!window) {
        printf("Failed to open %d x %d window: %s\n", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_GetError());
        exit(1);
    }

    // SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, rendererFlags);

    if (!renderer) {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_SetRenderDrawColor(renderer, 96, 128, 255, 255);
    SDL_RenderClear(renderer);

    while (1) {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    exit(0);
                    break;

                default:
                    break;
            }
        }

        SDL_RenderPresent(renderer);
    }

    SDL_Quit();
}

int main(int argc, char **argv) {
    Slideshow sh;

    handle_cli_args(argc, argv, &sh);

    if (read_entire_file(&sh) > 0) {
        // NOTE: Temp solution. Once UI is in, we can add a file selector.
        exit(1);
    }

    split_slides(&sh);

    if (STDOUT_MODE) {
        stdout_display(&sh);
    } else {
        // TODO: Integrate with SDL2

        // NOTE: This is a temp funtion while testing SDL2.
        sdl_test();

    }

    // NOTE: We probably should check if slides_content has been initialized?
    for (int i = 0; i < sh.slide_count; i++) {
        free(sh.slides_content[i]);
    }

    free(sh.slides_content);

    return 0;
}

void stdout_display(const Slideshow *sh) {
    bool done = false;
    int current_slide = 0;

    printf("[SLIDE #%d/%d]\n", current_slide + 1, sh->slide_count);
    for (size_t i = 0; i < sh->content_size; i++) {
        printf("%c", sh->slides_content[0][i]);
    }

    while (!done) {
        int user_input;

        enum input_handling {
            NEXT_SLIDE =  1,
            PREV_SLIDE = -1,
            QUIT       =  0,
            JUMP       =  8,
        };

        printf("\n");
        printf("-----------------------------------------------------------------\n");
        printf("Next Slide (1) | Prev. Slide (-1) | Jump to Slide (8) | Quit (0) > ");

        scanf("%d", &user_input);
        switch (user_input) {
            case NEXT_SLIDE:
                if ((current_slide + 1) < sh->slide_count) {
                    current_slide++;
                    printf("[SLIDE #%d/%d]\n", current_slide + 1, sh->slide_count);
                    for (size_t i = 0; i < sh->content_size; i++) {
                        printf("%c", sh->slides_content[current_slide][i]);
                    }
                } else {
                    current_slide = (sh->slide_count - 1);
                }
                break;
            case PREV_SLIDE:
                if (current_slide == 0) {
                    current_slide = 0;
                } else {
                    current_slide--;
                    printf("[SLIDE #%d/%d]\n", current_slide + 1, sh->slide_count);
                    for (size_t i = 0; i < sh->content_size; i++) {
                        printf("%c", sh->slides_content[current_slide][i]);
                    }
                }
                break;
            case QUIT:
                printf("Goodbye\n");
                done = true;
                break;
            case JUMP:
                // NOTE: If a user types an insanely big number, results are strange. But eh. Whatever.
                printf("Slide: (%d total slides) ", sh->slide_count);
                scanf("%d", &user_input);
                if (user_input < 0 || user_input == 0) {
                    current_slide = 0;
                } else if (user_input > (sh->slide_count - 1)) {
                    current_slide = (sh->slide_count - 1);
                } else current_slide = (user_input - 1);

                printf("[SLIDE #%d/%d]\n", current_slide + 1, sh->slide_count);
                for (size_t i = 0; i < sh->content_size; i++) {
                    printf("%c", sh->slides_content[current_slide][i]);
                }

                break;
            default:
                printf("ERROR: Bad input. Try again\n");
                break;
        }
    }
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
                            state = READY;
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
}

int read_entire_file(Slideshow *sh) {
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

    fclose(file);

    return 0;

cleanup:
    if (file) fclose(file);

    return 1;
}
