#include <vector>
#include "lodepng.h"
#include <iostream>

using std::vector;


#pragma once

class Image
{
public:
    vector<unsigned char> pixels;
    unsigned width;
    unsigned height;

    vector<vector<unsigned char>> red;
    vector<vector<unsigned char>> green;
    vector<vector<unsigned char>> blue;
    vector<vector<unsigned char>> alpha;

    Image(unsigned width, unsigned height)
    {
        this->width = width;
        this->height = height;
    }

    Image(const char* filename, unsigned width, unsigned height, bool splitIntoChannels)
    {
        this->width = width;
        this->height = height;

        unsigned error = lodepng::decode(pixels, width, height, filename);

        if (error)
            std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

        if (splitIntoChannels)
        {
            red.resize(height);
            green.resize(height);
            blue.resize(height);
            alpha.resize(height);

            for (size_t i = 0; i < height; i++)
            {
                red[i].reserve(width);
                green[i].reserve(width);
                blue[i].reserve(width);
                alpha[i].reserve(width);

                for (size_t j = 0; j < width * 4; j += 4)
                {
                    red[i].push_back(pixels[i * 4 * width + j]);
                    green[i].push_back(pixels[i * 4 * width + j + 1]);
                    blue[i].push_back(pixels[i * 4 * width + j + 2]);
                    alpha[i].push_back(pixels[i * 4 * width + j + 3]);
                }
            }
        }
    }

    void Save(const char* filename)
    {
        unsigned error = lodepng::encode(filename, pixels, width, height);
        if (error)
            std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
    }

};