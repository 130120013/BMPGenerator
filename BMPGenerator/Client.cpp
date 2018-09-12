#include "BMPGenerator.h"
#define TWO_PI 6.28
#define WIDTH 1920
#define HEIGHT 1080

double GetValue(unsigned x, unsigned y)
{
	return std::sin(((double) x * TWO_PI) / (WIDTH / 10)) * std::sin(((double) y * TWO_PI)/(HEIGHT / 10));
}

int main(int argc, char **argv)
{
	double(*GetVal)(unsigned, unsigned);
	GetVal = &GetValue;
	generateBMP("Test15.bmp", -1, 1, GetVal, WIDTH, HEIGHT, true);
	return 0;
}