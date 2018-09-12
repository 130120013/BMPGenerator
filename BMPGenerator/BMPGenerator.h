#include <stdio.h>
#include <stdlib.h>
#include <utility>
#include <cmath>
#include <cstdint>

#ifndef COMMON_BMPGENERATOR_H
#define COMMON_BMPGENERATOR_H

typedef struct __tagRGBTRIPLE
{
	std::uint8_t rgbBlue;
	std::uint8_t rgbGreen;
	std::uint8_t rgbRed;
} RGBTRIPLE;

bool generateBMP(char* name, double val_min, double val_max, double (*GetValue)(unsigned, unsigned), std::int32_t fWidth = 1500, std::int32_t fHeight = 200, bool fDiscardFileIfExists = true);

#endif
