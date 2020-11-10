# Waste
Waste is a simple program showing how many seconds of your life you have wasted, written in C using SDL2

## Compilation
* **Set up build environment**
	* **Linux**
		* Install needed software
			* Debian / Ubuntu
		  ```
		  # apt install build-essential git libsdl2-2.0-0 libsdl2-dev libsdl2-mixer-2.0-0 libsdl2-mixer-dev libsdl2-ttf-2.0-0 libsdl2-ttf-dev
		  ```
			* Arch / Manjaro
		  ```
		  # pacman -S base-devel git sdl2 sdl2_mixer sdl2_ttf
		  ```
	* **Windows**
		* Install [MSYS2](https://www.msys2.org/) and update it using instructions on its website
		* Open `MSYS2 MinGW 32-bit` from the Start menu
		* Install needed software
	  ```
	  $ pacman -S mingw-w64-i686-toolchain mingw-w64-i686-SDL2 mingw-w64-i686-SDL2_mixer  mingw-w64-i686-SDL2_ttf
	  ```
* **Clone this repository**
  ```
  $ git clone https://github.com/DieMaking/Waste.git
  ```
* **Compile and run**
  ```
  $ cd Waste
  $ make BUILD=release
  $ make run
  ```
