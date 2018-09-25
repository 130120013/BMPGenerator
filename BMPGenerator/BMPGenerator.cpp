#include <cstdio>
#include "BMPGenerator.h"

static inline unsigned scaleBetween(double unscaledNum, double min, double max) 
{
	return unsigned(MAX_COLORS * (unscaledNum - min) / (max - min)); //scale from [min; max) to [0; MAX_COLORS)
}

static bool ValToRGB(double nVal, double nMin, double nMax, RGBTRIPLE* colour)
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

struct bmp_file_handle_close
{
	inline void operator()(std::FILE* fp) const noexcept
	{
		std::fclose(fp);
	}
};

using unique_bmp_file_handle = std::unique_ptr<std::FILE, bmp_file_handle_close>;

unique_bmp_file_handle CreateBitmapFile(const char* name, std::uint32_t fWidth, std::uint32_t fHeight, bool fDiscardFileIfExists, uint32_t* cbPadding)
{
	auto fp = unique_bmp_file_handle(std::fopen(name, "r"));
	if (bool(fp) && !fDiscardFileIfExists)
			return unique_bmp_file_handle();

	fp.reset(std::fopen(name, "wb"));
	if (!fp)
		return unique_bmp_file_handle();

	std::uint8_t bfType[2] = { 'B','M' };
	std::fwrite(&bfType[0], 2 * BYTESIZE, 1, fp.get());

	*cbPadding = std::uint32_t(fWidth & 3);

	auto cbPaddedWidth = fWidth * 3 + *cbPadding;
	std::uint32_t bfSize = 54 + cbPaddedWidth * fHeight;
	std::fwrite(&bfSize, DWORDSIZE, 1, fp.get());

	// bfReserved1 + bfReserved2
	std::uint32_t reserved = 0;
	std::fwrite(&reserved, DWORDSIZE, 1, fp.get());

	std::uint32_t bfOffBits = 14;
	std::fwrite(&bfOffBits, DWORDSIZE, 1, fp.get());

	std::uint32_t biSize = 40;
	std::fwrite(&biSize, DWORDSIZE, 1, fp.get());

	std::uint32_t biWidth = fWidth;
	std::fwrite(&biWidth, LONGSIZE, 1, fp.get());

	std::uint32_t biHeight = fHeight;
	std::fwrite(&biHeight, LONGSIZE, 1, fp.get());

	std::uint16_t biPlanes = 1;
	std::fwrite(&biPlanes, WORDSIZE, 1, fp.get());

	std::uint16_t biBitCount = 24;
	std::fwrite(&biBitCount, WORDSIZE, 1, fp.get());

	std::uint32_t biCompression = 0; //without compression
	std::fwrite(&biCompression, DWORDSIZE, 1, fp.get());

	std::uint32_t biSizeImage = 0;
	std::fwrite(&biSizeImage, DWORDSIZE, 1, fp.get());

	std::uint32_t biXPelsPerMeter = 0;
	std::fwrite(&biXPelsPerMeter, LONGSIZE, 1, fp.get());

	std::uint32_t biYPelsPerMeter = 0;
	std::fwrite(&biYPelsPerMeter, LONGSIZE, 1, fp.get());

	std::uint32_t biClrUsed = 0;
	std::fwrite(&biClrUsed, DWORDSIZE, 1, fp.get());

	std::uint32_t biClrImportant = 0;
	std::fwrite(&biClrImportant, DWORDSIZE, 1, fp.get());

	return fp;
}

bool generateBMP(const char* name, const double* pData, std::uint32_t fWidth, std::uint32_t fHeight, double val_min, double val_max, bool fDiscardFileIfExists) 
{
	std::uint32_t cbPadding; 
	auto fp = CreateBitmapFile(name, fWidth, fHeight, fDiscardFileIfExists, &cbPadding);
	if (!fp)
		return false;

	static const std::uint32_t padding = 0;

	for (std::uint32_t l = 0; l < fHeight; ++l)
	{
		for (std::uint32_t k = 0; k < fWidth; ++k)
		{
			RGBTRIPLE rgb;
			bool successCode = ValToRGB(pData[k + l * fWidth], val_min, val_max, &rgb);
			if(successCode)
			{
				std::fwrite(&rgb.rgbBlue, 1, 1, fp.get());
				std::fwrite(&rgb.rgbGreen, 1, 1, fp.get());
				std::fwrite(&rgb.rgbRed, 1, 1, fp.get());
			}
		}
		std::fwrite(&padding, 1, cbPadding, fp.get());
	}
	return true;
}

bool generateBMP(const char* name, const double* pData, std::uint32_t fWidth, std::uint32_t fHeight, bool fDiscardFileIfExists)
{
	auto minmax = extreme2Task(&pData[0], &pData[fWidth * fHeight]);
	return generateBMP(name, pData, fWidth, fHeight, *minmax.first, *minmax.second, fDiscardFileIfExists);
}