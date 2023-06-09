#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>

// SDL STUFF
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "main.h"

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

void sdl_test(const Slideshow *sh) {
    // TODO: Move to a gui.h / gui.c file

    // Quick guide: https://www.geeksforgeeks.org/sdl-library-in-c-c-with-examples/

    int rendererFlags, windowFlags;

    rendererFlags = SDL_RENDERER_ACCELERATED;

    windowFlags = SDL_WINDOW_RESIZABLE;

    // TODO: #define instead of vars
    int SCREEN_WIDTH = 1280;
    int SCREEN_HEIGHT = 720;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Couldn't init SDL: %s\n", SDL_GetError());
        exit(1);
    }


    // TODO: structs for SDL_Window and SDL_Renderer?
    // Probably makes sense so we can easily pass these around to other funcs
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


    // START TEXT STUFF HERE
    // SEE: https://stackoverflow.com/questions/22886500/how-to-render-text-in-sdl2
    // This opens the font style and sets the size
    TTF_Init();
    TTF_Font* font = TTF_OpenFont("./OpenSans-Regular.ttf", 172);

    // set color in rgb format
    SDL_Color White = {255, 255, 255, 255};

    // create SDL surface
    SDL_Surface* surfaceMessage = TTF_RenderText_Solid(font, sh->slides_content[0], White);

    // convert the surface into a texture
    SDL_Texture* Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);

    // create a rect
    SDL_Rect window_rect;
    window_rect.x = 0;
    window_rect.y = 0;
    window_rect.w = 1280;
    window_rect.h = 720;

    // END TEXT STUFF

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    //SDL_RenderClear(renderer);

    while (1) {
        SDL_Event event;

        // NOTE: https://stackoverflow.com/questions/12770098/how-to-keep-the-cpu-usage-down-while-running-an-sdl-program
        if (SDL_WaitEvent(&event) != 0) {
            switch (event.type) {
                case SDL_QUIT:
                    exit(0);
                    break;

                // Left, right nav for slides. Typing J will start "Jump to slide"
                // ESC key is menu (if at menu ESC again, will prompt to quit)
                case SDL_KEYDOWN: {
                    switch (event.key.keysym.sym) {
                        case SDLK_LEFT:
                            printf("LEFT!\n");
                            break;
                        case SDLK_RIGHT:
                            printf("RIGHT!\n");
                            break;
                    }
                }
                break;
                default:
                    break;
            }
        }

        SDL_RenderPresent(renderer);

        SDL_RenderClear(renderer);
        // This should render the text?
        SDL_RenderCopy(renderer, Message, NULL, &window_rect);

        // TODO: Display each slide in the window as soon as the app is launched (for now --no file browser)
        // TODO: Left and right arrows change slide
        // TODO: Parse the slide markdown to format the contents
    }

    // Don't forget to free your surface and texture

    SDL_FreeSurface(surfaceMessage);
    SDL_DestroyTexture(Message);
    SDL_Quit();
}

int main(int argc, char **argv) {
    Slideshow sh;

    handle_cli_args(argc, argv, &sh);

    // TODO: File Explorer
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
        sdl_test(&sh);
    }


    if (sh.file_contents) {
        free(sh.file_contents);
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
        printf("CURRENT SLIDE: %d\n", current_slide);

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
    //
    //
    // TODO: Something seems to be up here? See gdb, sh.slides_content[1] and onwards
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
                        sh->slides_content[slide_count - 1][i] = '\0'; //This doesn't seem to do it?
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


    // NOTE: So basically, if I try to print the string (or use as a string) it fails.
    // However, If I print the chars in a loop, the data is there. Clearly I have to null terminate. Question is how and where.
    printf("CHAR ARRAY LOOP DEMO:-----\n");
    for (size_t i = 0; i < sh->content_size; i++) {
        char c = sh->slides_content[0][i];
        if (c == '\0') printf("\\0");
        printf("%c", c);
    }
    printf("\n");
    printf("END CHAR ARRAY LOOP DEMO \n");

    printf("STRING PRINT HERE\n");
    printf("%s", sh->slides_content[1]);
    printf("END STRING PRINT HERE\n");
/*
    printf(">>>>>>>>>\n");
    printf("%s", sh->slides_content[1]);
    printf(">>>>>>>>>\n");
    printf("\n");
*/
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
