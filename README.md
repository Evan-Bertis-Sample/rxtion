# rxtion

A custom game-engine & game, written in C99, inspired by Noita. It uses Gunslinger, an outsanding single-header library that abstracts away platform-specific code.

I'm using this project to get better with C (not C++!), particularly programming lower-level game engines.

You can also check the main **Gunslinger** repository here,
[Gunlinger repository](https://github.com/MrFrenik/gunslinger).
 
## Cloning instructions: 
- Use the following command to clone the repo and init the gunslinger submodule
```bash
git clone --recursive https://github.com/MrFrenik/gs_project_template
```

## Updating GS instructions (updates gunslinger submodule and rebases to main branch): 
```bash
git submodule update --remote --rebase --recursive
```

## Build Instructions:

*Note:* I have modified only the MINGW build script to work with my setup. The other build scripts are unmodified from the original Gunslinger project template. This means that the other build scripts may not work as expected.

For quick building and running for MINGW, use the following commands:
```
./qbt.sh # QUICK BUILD AND TEST
```

### Windows
## MSVC:
- From start menu, open `{x86|x64} Native Tools for {VS Version Here}`
- cd to `root dir` where you downloaded project template
- To compile the project, run:
```bash
proc\win\cl.bat
```
- To execute the program, run: 
```bash
bin\App.exe
```
### MINGW:
- From `root dir`, open `git bash`
- To compile, run: 
```bash
bash ./proc/win/mingw.sh
```
- To execute the program, run:
```bash
./bin/App.exe
```

### Linux

## Before Compiling:
- Make sure the following development libraries are installed: 
```bash
sudo apt install git gcc mesa-common-dev libxcursor-dev libxrandr-dev libxinerama-dev libxi-dev
```
- For Mesa and OpenGL, need to export the following: 
```bash
export MESA_GL_VERSION_OVERRIDE=3.3
```
- Credit to https://github.com/Samdal

## GCC:
- Open terminal
- cd to `root dir` where you downloaded project template
- To compile the project, run:
```bash
bash ./proc/linux/gcc.sh
```
- To execute the program, run: 
```bash
./bin/App
```

### OSX
## GCC: 
- Open terminal
- cd to `root dir` where you downloaded project template
- To compile the project, run:
```bash
bash ./proc/osx/gcc.sh
```
- To execute the program, run: 
```bash
./bin/App
```

### HTML5
## Emscripten: 
- Open terminal
- cd to `root dir` where you downloaded project template
- To compile the project, run:
```bash
bash ./proc/osx/emcc.sh
```
- This will generate the appropriate .html, .js, and .wsm files to load in a browser. 
