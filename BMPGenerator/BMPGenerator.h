#include <stdio.h>
#include <stdlib.h>
#include <utility>
#include <cmath>
#include <cstdint>
#include <future>
#include <thread>
#include <limits>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include <type_traits>

#ifndef COMMON_BMPGENERATOR_H
#define COMMON_BMPGENERATOR_H

typedef struct __tagRGBTRIPLE
{
	std::uint8_t rgbBlue;
	std::uint8_t rgbGreen;
	std::uint8_t rgbRed;
} RGBTRIPLE;

#define RANGES 8
#define RANGE_COLORS 255
#define LAST_RANGE_COLORS 1
#define MAX_COLORS ((RANGES - 1) * RANGE_COLORS + LAST_RANGE_COLORS)

#define LONGSIZE sizeof(std::int32_t) //4
#define BYTESIZE sizeof(std::uint8_t) //1
#define DWORDSIZE sizeof(std::uint32_t) //4
#define WORDSIZE sizeof(std::uint16_t) //2

//predicates
template <class Compare, class T>
struct impl_less_t
{
	constexpr bool operator()(const T& x, const T& y) const
	{
		return m_compare(x, y);
	}
	impl_less_t() = default;
	explicit constexpr impl_less_t(const Compare& compare) :m_compare(compare) {}
private:
	Compare m_compare;
};

template <class Compare, class T>
struct impl_equal_t
{
	constexpr bool operator()(const T& x, const T& y) const
	{
		return !m_compare(x, y) && !m_compare(y, x);
	}
	impl_equal_t() = default;
	explicit constexpr impl_equal_t(const Compare& compare) :m_compare(compare) {}
private:
	Compare m_compare;
};

template <class Compare, class T>
struct impl_less_equal_t
{
	constexpr bool operator()(const T& x, const T& y) const
	{
		return m_compare(x, y) || !m_compare(x, y) && !m_compare(y, x);
	}
	impl_less_equal_t() = default;
	explicit constexpr impl_less_equal_t(const Compare& compare) :m_compare(compare) {}
private:
	Compare m_compare;
};

template <class Compare, class T>
struct impl_greater_equal_t
{
	constexpr bool operator()(const T& x, const T& y) const
	{
		return !m_compare(x, y);
	}
	impl_greater_equal_t() = default;
	explicit constexpr impl_greater_equal_t(const Compare& compare) :m_compare(compare) {}
private:
	Compare m_compare;
};

template <class Compare, class T>
struct impl_greater_t
{
	constexpr bool operator()(const T& x, const T& y) const
	{
		return !m_compare(x, y) && (m_compare(x, y) || m_compare(y, x));
	}
	impl_greater_t() = default;
	explicit constexpr impl_greater_t(const Compare& compare) :m_compare(compare) {}
private:
	Compare m_compare;
};

template <class Compare, class T>
struct impl_not_equal_t
{
	constexpr bool operator()(const T& x, const T& y) const
	{
		return m_compare(x, y) || m_compare(y, x);
	}
	impl_not_equal_t() = default;
	explicit constexpr impl_not_equal_t(const Compare& compare) :m_compare(compare) {}
private:
	Compare m_compare;
};
//////////

template <class Iterator, class Compare>
Iterator extremeTask(Iterator arrBegin, Iterator arrEnd, const Compare& compare)
{
	if (arrBegin == arrEnd)
		return arrEnd;

	using value_type = typename std::iterator_traits<Iterator>::value_type;
	//constexpr auto PARALLEL_THRESHOLD = 2;
	unsigned threadCount = std::thread::hardware_concurrency();
	/*if (arrEnd - arrBegin < PARALLEL_THRESHOLD || threadCount < 2)
		return std::less<double>(arrBegin, arrEnd);*/
	std::vector<std::future<Iterator>> futures;
	futures.reserve(threadCount);
	int modulo = (arrEnd - arrBegin) % threadCount; 
	auto i = arrBegin;

	while (i < arrEnd)
	{
		std::int64_t part = (arrEnd - arrBegin) / threadCount; //count of elements in a thread
		if (modulo != 0)
			part += 1;
		auto iEnd = i + part;

		futures.emplace_back(std::async(std::launch::async, [](Iterator i, Iterator iEnd, Compare comp) -> Iterator
		{
			auto res = i;
			for (auto it = i; it < iEnd; ++it)
			{
				if(comp(*it, *res))
					res = it;
			}
			return res;
		}, i, iEnd, compare));

		if (modulo != 0)
			modulo -= 1;
		i += part;
	}
	auto it_extremum = futures[0].get();
	for (std::size_t iFut = 1; iFut < futures.size(); ++iFut)
	{
		auto it_val = futures[iFut].get();
		if (compare(*it_val, *it_extremum))
			it_extremum = it_val;
	}
	return it_extremum;
}

template <class Iterator, class CompareLess, class CompareGreater>
std::pair<Iterator, Iterator> extreme2Task(Iterator arrBegin, Iterator arrEnd, const CompareLess& less, const CompareGreater& greater)
{
		if (arrBegin == arrEnd)
		return std::pair<Iterator, Iterator>(arrEnd, arrEnd);

	using value_type = typename std::iterator_traits<Iterator>::value_type;
	//constexpr auto PARALLEL_THRESHOLD = 2;
	unsigned threadCount = std::thread::hardware_concurrency();
	/*if (arrEnd - arrBegin < PARALLEL_THRESHOLD || threadCount < 2)
		return std::less<double>(arrBegin, arrEnd);*/
	std::vector<std::future<std::pair<Iterator, Iterator>>> futures;
	futures.reserve(threadCount);
	int modulo = (arrEnd - arrBegin) % threadCount;
	auto i = arrBegin;

	while (i < arrEnd)
	{
		std::int64_t part = (arrEnd - arrBegin) / threadCount;  //count of elements in a thread
		if (modulo != 0)
			part += 1;
		auto iEnd = i + part;

		futures.emplace_back(std::async(std::launch::async, [greater](Iterator i, Iterator iEnd, CompareLess compLess) -> std::pair<Iterator, Iterator>
		{
			std::pair<Iterator, Iterator> res = std::pair<Iterator, Iterator>(i, i);
			for (auto it = i; it < iEnd; ++it)
			{
				if(compLess(*it, *res.first))
					res.first = it;
				if(greater(*it, *res.second))
					res.second = it;
			}
			return res;
		}, i, iEnd, less));

		if (modulo != 0)
			modulo -= 1;
		i += part;
	}
	auto it_extremum = futures[0].get();
	for (std::size_t iFut = 1; iFut < futures.size(); ++iFut)
	{
		auto it_val = futures[iFut].get();
		if(less(*it_val.first, *it_extremum.first))
			it_extremum.first = it_val.first;

		if(greater(*it_val.second, *it_extremum.second))
					it_extremum.second = it_val.second;
	}
	return it_extremum;
}

template <class Iterator, class CompareLess>
auto extreme2Task(Iterator arrBegin, Iterator arrEnd, const CompareLess& less)
{
	return extreme2Task(arrBegin, arrEnd, less, impl_greater_t<CompareLess, double>(less));
}

template <class Iterator>
auto extreme2Task(Iterator arrBegin, Iterator arrEnd)
{
	return extreme2Task(arrBegin, arrEnd, std::less<typename std::iterator_traits<Iterator>::value_type>());
}

inline unsigned scaleBetween(const double unscaledNum, const double min, const double max) 
{
	return unsigned(MAX_COLORS * (unscaledNum - min) / (max - min)); //scale from [min; max) to [0; MAX_COLORS)
}

template <class Iterator>
void updateHeights(Iterator begin, Iterator end)
{
	for (auto it = begin; it != end; ++it)
	{
		if (*it < -500)
			*it = 0;
	}
}

bool ValToRGB(const double nVal, const double nMin, const double nMax, RGBTRIPLE* colour); //converts a value to a RGB colour
FILE* CreateBitmapFile(const char* name, const std::int32_t fWidth, const std::int32_t fHeight, const bool fDiscardFileIfExists, int32_t* cbPadding); //creates a file with BMP headers

template <class Callable>
bool generateBMP(const char* name, const double val_min, const double val_max, const Callable&& GetValue, const std::int32_t fWidth, const std::int32_t fHeight, const bool fDiscardFileIfExists) 
{
	std::int32_t cbPadding; 
	FILE* fp = CreateBitmapFile(name, fWidth, fHeight, fDiscardFileIfExists, &cbPadding);
	if (!fp)
		return false;

	static const std::uint32_t padding = 0;

	for (int l = 0; l < fHeight; ++l)
	{
		for (int k = 0; k < fWidth; ++k)
		{
			RGBTRIPLE rgb;
			bool successCode = ValToRGB(GetValue(k, l), val_min, val_max, &rgb);
			if(successCode)
			{
				fwrite(&rgb.rgbBlue, 1, 1, fp);
				fwrite(&rgb.rgbGreen, 1, 1, fp);
				fwrite(&rgb.rgbRed, 1, 1, fp);
			}
		}
		fwrite(&padding, 1, cbPadding, fp);
	}
	fclose(fp);
	delete fp;
	return true;
}

template <class Callable>
bool generateBMP(const char* name, const Callable&& GetValue, const std::int32_t fWidth, const std::int32_t fHeight, const bool fDiscardFileIfExists)
{
	std::int32_t cbPadding;
	FILE* fp = CreateBitmapFile(name, fWidth, fHeight, fDiscardFileIfExists, &cbPadding);
	if (!fp)
		return false;

	static const std::uint32_t padding = 0;
	auto arrSize = fWidth * fHeight;
	double* heightmatrix = new double[arrSize];

	for (int l = 0; l < fHeight; ++l)
	{
		for (int k = 0; k < fWidth; ++k)
		{
			heightmatrix[l*fWidth + k] = GetValue(k, l);
		}
	}

	auto minmaxVal = extreme2Task(heightmatrix, heightmatrix + arrSize, std::less<double>());
	if (minmaxVal.first == heightmatrix + arrSize)
		return false;

	for (int l = 0; l < fHeight; ++l)
	{
		for (int k = 0; k < fWidth; ++k)
		{
			RGBTRIPLE rgb;
			bool successCode = ValToRGB(GetValue(k, l), *minmaxVal.first, *minmaxVal.second, &rgb);
			if (successCode)
			{
				fwrite(&rgb.rgbBlue, 1, 1, fp);
				fwrite(&rgb.rgbGreen, 1, 1, fp);
				fwrite(&rgb.rgbRed, 1, 1, fp);
			}
		}
		fwrite(&padding, 1, cbPadding, fp);
	}
	fclose(fp);
	return true;
}

template <class Callable>
bool generateBMP(const char* name, const Callable&& GetValue, int16_t* data, const std::int32_t fWidth, const std::int32_t fHeight, const bool fDiscardFileIfExists)
{
	std::int32_t cbPadding;
	FILE* fp = CreateBitmapFile(name, fWidth, fHeight, fDiscardFileIfExists, &cbPadding);
	if (!fp)
		return false;

	static const std::uint32_t padding = 0;
	auto arrSize = fWidth * fHeight;
	//double* heightmatrix = new double[arrSize];

	//for (int l = 0; l < fHeight; ++l)
	//{
	//	for (int k = 0; k < fWidth; ++k)
	//	{
	//		heightmatrix[l*fWidth + k] = GetValue(k, l);
	//	}
	//}

	updateHeights(data, data + arrSize);

	auto minmaxVal = extreme2Task(data, data + arrSize, std::less<int16_t>());
	if (minmaxVal.first == data + arrSize)
		return false;

	for (int l = 0; l < fHeight; ++l)
	{
		for (int k = 0; k < fWidth; ++k)
		{
			RGBTRIPLE rgb;
			bool successCode = ValToRGB(GetValue(k, l), *minmaxVal.first, *minmaxVal.second, &rgb);
			if (successCode)
			{
				fwrite(&rgb.rgbBlue, 1, 1, fp);
				fwrite(&rgb.rgbGreen, 1, 1, fp);
				fwrite(&rgb.rgbRed, 1, 1, fp);
			}
		}
		fwrite(&padding, 1, cbPadding, fp);
	}
	fclose(fp);
	return true;
}
#endif
