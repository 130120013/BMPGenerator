#include "BMPGenerator.h"
#include <iostream>
#define TWO_PI 6.28

int main(int argc, char **argv)
{
	std::cout << "Enter the desired width and height: ";
	unsigned nWidth, nHeight;
	std::cin >> nWidth >> nHeight;
	generateBMP("Test15.bmp", 0, 2, [nWidth, nHeight] (auto x, auto y) -> auto 
	{
		double scaled_x = ((double) x) / (nWidth) - 0.5;
		double scaled_y = ((double) y) / (nHeight) - 0.5;
		return scaled_x * scaled_x + scaled_y * scaled_y;
	}, nWidth, nHeight, true);
	return 0;
}