#include "Image.h"

#pragma once

class ImageRotation
{
public:
	static Image RotateImage(Image image, int degrees);
	static Image FastRotateImage(Image image, int degrees);
};