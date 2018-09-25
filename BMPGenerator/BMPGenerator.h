#include <cstdlib>
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
			for (auto it = std::next(i); it < iEnd; ++it)
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

bool generateBMP(const char* name, const double* pData, std::uint32_t fWidth, std::uint32_t fHeight, double val_min, double val_max, bool fDiscardFileIfExists);

bool generateBMP(const char* name, const double* pData, std::uint32_t fWidth, std::uint32_t fHeight, bool fDiscardFileIfExists);

template <class Callable, class = decltype(std::declval<Callable&&>()(std::declval<std::uint32_t>(), std::declval<std::uint32_t>()))>
bool generateBMP(const char* name, double val_min, double val_max, Callable&& GetValue, std::uint32_t fWidth, std::uint32_t fHeight, bool fDiscardFileIfExists) 
{
	auto heightmatrix = std::make_unique<double[]>(fWidth * fHeight);

	for (std::uint32_t l = 0; l < fHeight; ++l)
		for (std::uint32_t k = 0; k < fWidth; ++k)
			heightmatrix[l*fWidth + k] = GetValue(k, l);

	return generateBMP(name, heightmatrix.get(), fWidth, fHeight, val_min, val_max, fDiscardFileIfExists);
}

template <class Callable, class = decltype(std::declval<Callable&&>()(std::declval<std::uint32_t>(), std::declval<std::uint32_t>()))>
bool generateBMP(const char* name, Callable&& GetValue, std::uint32_t fWidth, std::uint32_t fHeight, bool fDiscardFileIfExists)
{
	auto heightmatrix = std::make_unique<double[]>(fWidth * fHeight);

	for (std::uint32_t l = 0; l < fHeight; ++l)
		for (std::uint32_t k = 0; k < fWidth; ++k)
			heightmatrix[l*fWidth + k] = GetValue(k, l);

	return generateBMP(name, heightmatrix.get(), fWidth, fHeight, fDiscardFileIfExists);
}

#endif
