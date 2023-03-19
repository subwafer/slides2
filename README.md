# Slides2 (name needed)

A slideshow application in C made for fun and practice.

Inspired by [Jonathan Blow's video series](https://www.youtube.com/playlist?list=PLmV5I2fxaiCL9mYvYjUj0qTnYlK4qDhkn)

## Goals

User creates some sort of file representing a slideshow (.md for now). It is split into *slides*. User can navigate the slideshow in a gui.

## Features

- [x] reads a .md file
- [x] splits the file into slides (delimiter: "---')
- [x] basic display via stdout
- [x] user controls in stdout
- [ ] SDL2 gui

## Usage

``` shell
# build with build.sh

./build.sh

# run program in debug mode with stdout display
./main --debug --stdout
```

## Resources

- [Effective C](https://nostarch.com/Effective_C)
- [Tsoding Mini excel](https://www.youtube.com/playlist?list=PLpM-Dvs8t0VYfQc5dq21Vc81G1rGHwkmT)
