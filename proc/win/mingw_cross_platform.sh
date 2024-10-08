#!bin/bash

# Make sure you have mingw-w64 installed for this to work.
# It should be available in most linux package managers and with brew(MacOS).

rm -rf bin
mkdir bin
cd bin

proj_name=App
proj_root_dir=$(pwd)/../

flags=(
    -std=gnu99 -w
)

# Include directories
inc=(
    -I ../vendor/      # Gunslinger includes
)

# Source files
src=(
    ../source/main.c
)

libs=(
    -lopengl32
    -lkernel32
    -luser32
    -lshell32
    -lgdi32
    -lwinmm
)

# Build
x86_64-w64-mingw32-gcc -O0 ${inc[*]} ${src[*]} ${flags[*]} ${libs[*]} -lm -o ${proj_name}

cd ..
