# Hide The Data

PNG steganography in C with zlib. [Learn more about Steganography here.](https://en.wikipedia.org/wiki/Steganography) 

## Why

Heard about it. Sounded cool. Wanted to explore the concept for myself.

## Project Status

:white_check_mark: What Works

- Embedding strings in PNG images
- Retrieving embedded strings from PNG images

:construction: In Progress

- Embedding files in PNGs
- Adding support for options (e.g. -f)

:bug: Known Issues

- Extrememly long strings overwrite filter type bytes in scanlines, except for the first scanline.

:bulb: Future Plans

- Maintenance and optimisations 

> [!NOTE]
> There is currently no support for PNG images with interlacing.

## Usage

Hiding string
```
stego <image> h "<your input string>"
```

Extracting string
```
stego <image> e 
```

## Building

Requirements

- A C compiler
- CMake

Download zlib

Visit [zlib's Github Repository](https://github.com/madler/zlib) and download zlib.

Build zlib

Open the terminal in the zlib directory and run the following commands.

```
cmake -S . -B build -D CMAKE_BUILD_TYPE=Release
```

```
cmake --build build
```

Build Steganography

```
cmake -B build -DZLIB_ROOT=<path_to_zlib_directory>
```

```
cmake --build build
```

## How

The program uses a technique known as Least Significant Bit(s) (LSB) steganography to hide strings inside PNG images. This works by only modyfing the two least significant bits of each of the bytes of the image's pixel data. For every byte in the data we wish to hide, each pair of bits is extracted (byte has 8 bits, thus 4 pairs of bits). The two least significant bits in each byte of the image's pixel data is then set to these bits. The length of the string is hidden among the pixel data before the actual string that is being hidden, so that the program knows exactly how much of the image data it needs to process. 

## Feedback

I've only been working with C/C++ for a few years and am still fairly new to CMake, so I'd love to hear thoughts! If you find any bugs or have any suggestions, feel free to email me at keyboardslayer7@protonmail.com. Any feedback, big or small, is greatly appreciated!
