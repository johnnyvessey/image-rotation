# image-rotation
Accurate rotation of PNG images in C++

Requires lodepng.h and lodepng.cpp to be in the same directory, which are from the repo: https://github.com/lvandeve/lodepng

This project shows two different ways of rotating a PNG image in C++
These algorithms are both RAM-based (Rotation by Area Mapping), so they should produce accurate and smooth rotations, at the expense of
some performance compared to more approximate rotation algorithms.
