#!bin/bash

rm -rf bin
mkdir bin
cd bin

proj_name=rxtion
proj_root_dir=$(pwd)/../

flags=(
	-std=gnu99 -w -ggdb -D RXCORE_PROFILING_ENABLED
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

# # Preprocess main.c and output to a file
# main_src="../src/main.c"
# preprocessed_output="../preprocessed_main.i"
# preprocess_cmd="gcc -E ${inc[*]} $main_src ${flags[*]} -o $preprocessed_output"

# echo " "

# echo "Preprocessing main.c..."
# echo $preprocess_cmd
# $preprocess_cmd

# Build
build_cmd="gcc -O0 ${inc[*]} ${src[*]} ${flags[*]} ${libs[*]} -lm -o ${proj_name}"

echo "Building..."
echo $build_cmd
$build_cmd

cd ..


