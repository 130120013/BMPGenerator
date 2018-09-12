#include "BMPGenerator.h"

bool ValToRGB(double nVal, double nMin, double nMax, RGBTRIPLE* colour)
{
	unsigned x;
	unsigned nRange;
	std::uint8_t nIntVal;
	double nValRange;

	if (nMax < nMin)
		return ValToRGB(nVal, nMax, nMin, colour);
	nValRange = nMax - nMin;
	if (!nValRange) //i.e. nMax == 0xffffffffffffffff and nMin == 0
		return false;
	if (nVal <= nMin)
	{
		colour->rgbBlue = colour->rgbGreen = colour->rgbRed = 0x00;
		return true;
	}
	if (nVal >= nMax)
	{
		colour->rgbBlue = colour->rgbGreen = colour->rgbRed = 0xff;
		return true;
	}

	x = scaleBetween(nVal, nMin, nMax);
	if (x < MAX_COLORS - LAST_RANGE_COLORS)
	{
		nRange = (unsigned int)x / RANGE_COLORS;
		nIntVal = x % RANGE_COLORS;
	}
	else
	{
		colour->rgbBlue = colour->rgbGreen = colour->rgbRed = 0xff;
		return true;
	}

	switch (nRange)
	{
	case 0:
	{
		colour->rgbBlue = nIntVal;
		colour->rgbGreen = 0;
		colour->rgbRed = 0;
		return true;
	}
	case 1:
	{
		colour->rgbBlue = 0xff;
		colour->rgbGreen = nIntVal;
		colour->rgbRed = 0;
		return true;
	}
	case 2:
	{
		colour->rgbBlue = 0xff - nIntVal;
		colour->rgbGreen = 0xff;
		colour->rgbRed = 0;
		return true;
	}
	case 3:
	{
		colour->rgbBlue = 0;
		colour->rgbGreen = 0xff;
		colour->rgbRed = nIntVal;
		return true;
	}
	case 4:
	{
		colour->rgbBlue = 0;
		colour->rgbGreen = 0xff - nIntVal;
		colour->rgbRed = 0xff;
		return true;
	}
	case 5:
	{
		colour->rgbBlue = nIntVal;
		colour->rgbGreen = 0;
		colour->rgbRed = 0xff;
		return true;
	}
	case 6:
	{
		colour->rgbBlue = 0xff;
		colour->rgbGreen = nIntVal;
		colour->rgbRed = 0xff;
		return true;
	}
	default:
		return false;
	}
}