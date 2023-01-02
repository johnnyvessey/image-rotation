#include "ImageRotation.h"
#include <math.h>


vector<vector<unsigned char>> RotateChannel(vector<vector<unsigned char>> pixels, int degrees, size_t subPixelDensity)
{
	size_t width = pixels[0].size();
	size_t height = pixels.size();

	vector<vector<unsigned char>> newPixels = pixels;
	const double PI = 3.1415926535;
	double cosAngle = cos(degrees * PI / 180.0);
	double sinAngle = sin(degrees * PI / 180.0);

	float reciprocalSubPixelDensity = 1.0f / (float)subPixelDensity;
	float halfReciprocalSubPixelDensity = reciprocalSubPixelDensity / 2.0;

	for (size_t i = 0; i < height; i++)
	{
		for (size_t j = 0; j < width; j++)
		{
			float xl = (float)j - width / 2;
			float yt = (float)height / 2 - i;

			int avgColor = 0;
			for (size_t sub_x_idx = 0; sub_x_idx < subPixelDensity; sub_x_idx++)
			{
				for (size_t sub_y_idx = 0; sub_y_idx < subPixelDensity; sub_y_idx++)
				{
					float x = xl +  reciprocalSubPixelDensity * sub_x_idx + halfReciprocalSubPixelDensity; //offset by to make it the center of the grid
					float y = yt - reciprocalSubPixelDensity * sub_y_idx + halfReciprocalSubPixelDensity;

					int rotCol = (int)(x * cosAngle - y * sinAngle) + width / 2;
					int rotRow = height/2 - (int)(x * sinAngle + y * cosAngle);

					if (rotCol >= 0 && rotCol < width && rotRow >= 0 && rotRow < height)
					{
						avgColor += pixels[rotRow][rotCol];
					}
				}
			}
			//cast to int first to avoid weird float arithmetic causing average to be higher than 255 (overflow)
			newPixels[i][j] = static_cast<unsigned char>((int)((float)avgColor / (subPixelDensity * subPixelDensity)));
		}
	}

	return newPixels;
}

Image ImageRotation::RotateImage(Image image, int degrees)
{
	//iterate through the new image, pixel by pixel
	//rotate in the opposite direction to find which pixel would have ended up at your current pixel
	//if outside the bounds of the image, set all values to be 0 (since it's a png, it be have alpha of 0 and be clear)
	/*
	since rotated pixel will not line up will normal pixel(unless it's a multiple of 90 degrees),
	you need to rotate the four corners of each pixel, to create a rotated square
	then find what proportion of the area of that rotated pixel lies in each of the surrounding pixels (or is empty)
	then take a weighted average of each of those surrounding pixels to get the final result
	to estimate the area, sub-divide each pixel into a grid, then figure out how many of those sub-pixels are in each
	nearby pixel
	*/
	const size_t subPixelDensity = 4;

	Image newImage(image.width, image.height);
	newImage.red = RotateChannel(image.red, degrees, subPixelDensity);
	newImage.green = RotateChannel(image.green, degrees, subPixelDensity);
	newImage.blue = RotateChannel(image.blue, degrees, subPixelDensity);
	newImage.alpha = RotateChannel(image.alpha, degrees, subPixelDensity);

	newImage.pixels.reserve(4 * image.width * image.height);
	for (size_t i = 0; i < image.height; i++)
	{
		for (size_t j = 0; j < image.width;j++)
		{
			newImage.pixels.push_back(newImage.red[i][j]);
			newImage.pixels.push_back(newImage.green[i][j]);
			newImage.pixels.push_back(newImage.blue[i][j]);
			newImage.pixels.push_back(newImage.alpha[i][j]);
		}
	}
	return newImage;
}

/*
	This function is a much faster rotation algorithm.
	It does not rotate the image separately by channel, but rotates the pixels of the image
	to find what proportion of the rotated pixel lies in the neighboring pixels,
	then uses that proportion to find the weighted average of the nearby pixels for each of the colors of that pixel.
	This means we have to do a lot less calculations of the rotations.

	Also, in this algorithm, the corners of a pixel are not rotated, just the center of the pixel, and then a non-rotated pixel
	is used as the new pixel and is centered on the rotated center of the original pixel
	This will be slightly inaccurate because the new pixel should be a rotated square, but this is still accurate and much faster
*/
Image ImageRotation::FastRotateImage(Image image, int degrees)
{
	int width = (int)image.width;
	int height = (int)image.height;
	Image newImage(width, height);
	newImage.pixels.resize(width * height * 4);

	const double PI = 3.1415926535;
	double cosAngle = cos(PI * degrees  / 180.0);
	double sinAngle = sin(PI * degrees / 180.0);

	int neighborPixelCoefs[2][2];
	for (int i = 0; i < newImage.pixels.size(); i+= 4)
	{
		int row = (i / 4) / width;
		int col = (i / 4) % width;

		//rotate point at the center of each pixel
		float x = .5f + (float)col - width / 2;
		float y = (float)height / 2 - row - .5f;

		float newX = x * cosAngle - y * sinAngle -.5f;
		float newY = x * sinAngle + y * cosAngle + .5f;
		
		int newCol = (int)(newX + (float)width / 2);
		int newRow = (int)(-newY + (float)height / 2);

		if (newCol < 0 || newRow < 0 || newRow > image.height || newCol > image.width) continue;

		//reset neighborPixelCoefs
		neighborPixelCoefs[0][0] = 0; neighborPixelCoefs[0][1] = 0; neighborPixelCoefs[1][0] = 0; neighborPixelCoefs[1][1] = 0;

		//sample 16 points inside the new pixel to see which of them fall into specific neighboring pixels
		for (int y_idx = 0; y_idx < 4; y_idx++)
		{
			for (int x_idx = 0; x_idx < 4; x_idx++)
			{
				//offset by 1/8 because we want the sampling be centered on the pixel, not justified to the top-left
				int subPxlCol = (int)(.125f + newX + .25f * x_idx  + width / 2);
				int subPxlRow = (int)((int)image.height / 2 -(-.125f + newY - .25f * y_idx ));
				if (0 <= subPxlRow && subPxlRow < height && 0 <= subPxlCol && subPxlCol < width)
				{					
					neighborPixelCoefs[subPxlRow - newRow][subPxlCol - newCol]++;					
				}
			}
		}


		//loop through each color and set the value based on the proportions of neighboring pixels that should be sampled from
		for (size_t color = 0; color < 4; color++)
		{
			int colorValue = 0;
			for (size_t n_i = 0; n_i < 2; n_i++)
			{
				for (size_t n_j = 0; n_j < 2; n_j++)
				{
					int pixel_idx = 4 * ((newRow - 1 + n_i) * width + newCol - 1 + n_j) + color;
					if (pixel_idx >= 0 && pixel_idx < image.pixels.size())
					{
						colorValue += (neighborPixelCoefs[n_i][n_j] * image.pixels[pixel_idx]);
					}
					
				}
			}

			newImage.pixels[i + color] = static_cast<unsigned char>(colorValue / 16);
		}
	}

	return newImage;
}