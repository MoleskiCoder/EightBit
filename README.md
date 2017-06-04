## Invaders

[![Build Status](https://travis-ci.org/MoleskiCoder/invaders.svg?branch=master)](https://travis-ci.org/MoleskiCoder/invaders)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/12171/badge.svg)](https://scan.coverity.com/projects/moleskicoder-invaders)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/b40b739726bd410186f700546b40e604)](https://www.codacy.com/app/MoleskiCoder/invaders?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=MoleskiCoder/invaders&amp;utm_campaign=Badge_Grade)

Space Invaders Emulator written in C++

Uses SDL2 to provide graphics and SDL_mixer for sound effects.

### Features

* 2 player controls
* Coloured gel screen
* Sound effects
* Event driven Intel 8080 emulator
* Basic CP/M emulation (enough to run CPU tests)
* Intel 8080 profiler
* Intel 8080 disassembler

### To be done

* AppVeyor integration
* 8080 test cases. Maybe...

## Compiling

### Windows

Compile with Visual Studio 2015 (via the solution)

#### Prerequisites

* SDL_Mixer
* Visual Studio 2015

### Linux

* make opt
* make debug
* make coverage

#### Prerequisites

apt-get install build-essential libsdl2-dev libsdl2-mixer-dev

## Running

src/invaders

### Keyboard Controls

* 3: Insert Coin
* 1: 1P Start
* 2: 2P Start
* z: Left 1P
* x: Right 1P
* \\: Fire 1P
* ,: Left 2P
* .: Right 2P
* /: Fire 2P

### XBox360 Controller

* Left Bumper: Left
* Right Bumper: Right
* A Button: Fire

