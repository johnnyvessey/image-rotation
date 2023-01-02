#include "ImageRotation.h"
#include <chrono>

using namespace std::chrono;

int main()
{
	Image image("test_photo.png", 1024, 1024, false);
	Image rotatedImage = ImageRotation::FastRotateImage(image, 36);
	rotatedImage.Save("rotated_test_photo.png");
}

