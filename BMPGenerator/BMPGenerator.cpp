#include "BMPGenerator.h"

bool ValToRGB(const double nVal, const double nMin, const double nMax, RGBTRIPLE* colour)
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

FILE* CreateBitmapFile(const char* name, const std::int32_t fWidth, const std::int32_t fHeight, const bool fDiscardFileIfExists, int32_t* cbPadding)
{
	FILE *fp;
	if (fp = fopen(name, "r"))
	{
		if (!fDiscardFileIfExists)
			return NULL;
		fclose(fp);
	}

	fp = fopen(name, "wb");
	if (!fp)
		return NULL;

	std::uint8_t bfType[2] = { 'B','M' };
	fwrite(&bfType[0], 2 * BYTESIZE, 1, fp);

	*cbPadding = std::int32_t(fWidth & 3);

	auto cbPaddedWidth = fWidth * 3 + *cbPadding;
	std::uint32_t bfSize = 54 + cbPaddedWidth * fHeight;
	fwrite(&bfSize, DWORDSIZE, 1, fp);

	// bfReserved1 + bfReserved2
	std::uint32_t reserved = 0;
	fwrite(&reserved, DWORDSIZE, 1, fp);

	std::uint32_t bfOffBits = 14;
	fwrite(&bfOffBits, DWORDSIZE, 1, fp);

	std::uint32_t biSize = 40;
	fwrite(&biSize, DWORDSIZE, 1, fp);

	std::int32_t biWidth = fWidth;
	fwrite(&biWidth, LONGSIZE, 1, fp);

	std::int32_t biHeight = fHeight;
	fwrite(&biHeight, LONGSIZE, 1, fp);

	std::uint16_t biPlanes = 1;
	fwrite(&biPlanes, WORDSIZE, 1, fp);

	std::uint16_t biBitCount = 24;
	fwrite(&biBitCount, WORDSIZE, 1, fp);

	std::uint32_t biCompression = 0; //without compression
	fwrite(&biCompression, DWORDSIZE, 1, fp);

	std::uint32_t biSizeImage = 0;
	fwrite(&biSizeImage, DWORDSIZE, 1, fp);

	std::int32_t biXPelsPerMeter = 0;
	fwrite(&biXPelsPerMeter, LONGSIZE, 1, fp);

	std::int32_t biYPelsPerMeter = 0;
	fwrite(&biYPelsPerMeter, LONGSIZE, 1, fp);

	std::uint32_t biClrUsed = 0;
	fwrite(&biClrUsed, DWORDSIZE, 1, fp);

	std::uint32_t biClrImportant = 0;
	fwrite(&biClrImportant, DWORDSIZE, 1, fp);

	return fp;
}