#!bin/bash

rm -rf bin
mkdir bin
cd bin

proj_name=rxtion
proj_root_dir=$(pwd)/../

flags=(
	-std=gnu99 -w -ggdb
)

# Include directories
inc=(
	-I ../vendor/						# Vendor includes
	-I ../rxtion/						# Rxtion includes
)

# Automatically find C source files in specified directories
src=($(find ../src ../rxtion -type f -name '*.c'))

libs=(
	-lopengl32
	-lkernel32 
	-luser32 
	-lshell32 
	-lgdi32 
    -lWinmm
	-lAdvapi32
)

# Build
build_cmd="gcc -O0 ${inc[*]} ${src[*]} ${flags[*]} ${libs[*]} -lm -o ${proj_name}"

echo "Building..."
echo $build_cmd
$build_cmd

cd ..



