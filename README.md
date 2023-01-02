# image-rotation
Accurate rotation of PNG images in C++

Requires lodepng.h and lodepng.cpp to be in the same directory, which are from the repo: https://github.com/lvandeve/lodepng
(These files handle parsing PNG files into a vector of the RGBA values of the image)

This project shows two different ways of rotating a PNG image in C++
These algorithms are both RAM-based (Rotation by Area Mapping), so they should produce accurate and smooth rotations, at the expense of
some performance compared to more approximate rotation algorithms.
