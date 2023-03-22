# Slides2 (name needed)

A slideshow application in C made for fun and practice. It's incomplete. Use at your own risk.

Inspired by [Jonathan Blow's video series](https://www.youtube.com/playlist?list=PLmV5I2fxaiCL9mYvYjUj0qTnYlK4qDhkn)

## Goals

User creates some sort of file representing a slideshow (.md for now). It is split into *slides*. User can navigate the slideshow in a gui.

## Features

- [x] reads a .md file
- [x] splits the file into slides (delimiter: "---')
- [x] basic display via stdout
- [x] user controls in stdout
- [x] cli args handing for --file_path
- [ ] SDL2 gui
- [ ] file browser
- [ ] customize each slide
- [ ] customize slideshow with template

## Usage

``` shell
# build with build.sh

./build.sh

# run program in demo mode with SDL2 gui (WIP)
./main --demo

# run program in demo mode with stdout display
./main --demo --stdout

# run program and pass file_path with stdout display
./main --file_path="./demos/show1.md" --stdout
```

## Resources

- [Effective C](https://nostarch.com/Effective_C)
- [Tsoding Mini excel](https://www.youtube.com/playlist?list=PLpM-Dvs8t0VYfQc5dq21Vc81G1rGHwkmT)
- [C substring slicing](https://stackoverflow.com/questions/26620388/c-substrings-c-string-slicing)
