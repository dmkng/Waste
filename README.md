# Waste
Waste is a simple program showing how many seconds of your life you have wasted, written in C using SDL2. This project serves me as a kind of template for any new simple C or C++ apps I create, it has a very flexible, universal Makefile.

## Compilation

### 1. Set up build environment

#### Linux
* Install needed software (run as root or use `sudo` / `su` / `doas`)
	* Debian-based (Ubuntu / Linux Mint / Pop!_OS)
	```
	# apt update && apt install build-essential git libsdl2-2.0-0 libsdl2-dev libsdl2-mixer-2.0-0 libsdl2-mixer-dev libsdl2-ttf-2.0-0 libsdl2-ttf-dev
	```
	* Arch-based (EndeavourOS / Manjaro)
	```
	# pacman -Syu && pacman -S base-devel git sdl2 sdl2_mixer sdl2_ttf
	```

#### Windows
* If you don't have MSYS2 installed, download it from [www.msys2.org](https://www.msys2.org/) and install it
* Open `MSYS2 MinGW 64-bit` from the Start menu
* Update MSYS2 using instructions on [www.msys2.org/docs/updating/](https://www.msys2.org/docs/updating/)
* Install needed software
	```
	$ pacman -S mingw-w64-x86_64-toolchain git mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_mixer mingw-w64-x86_64-SDL2_ttf
	```

### 2. Clone this repository
	$ git clone https://github.com/dmkng/Waste.git

### 3. Compile and run
	$ cd Waste
	$ make RELEASE=1
	$ make run
