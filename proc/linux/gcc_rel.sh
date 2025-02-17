#!/bin/bash

rm -rf bin
mkdir bin
cd bin

proj_name=App
proj_root_dir=$(pwd)/../

flags=(
	-std=gnu99 -w -ldl -lGL -lX11 -pthread -lXi
)

# Include directories
inc=(
	-I ../vendor/
)

# Source files
src=(
	../source/main.c
)

# Build
echo gcc -O3 ${inc[*]} ${src[*]} ${flags[*]} -lm -o ${proj_name}
gcc -O3 ${inc[*]} ${src[*]} ${flags[*]} -lm -o ${proj_name}

cd ..
