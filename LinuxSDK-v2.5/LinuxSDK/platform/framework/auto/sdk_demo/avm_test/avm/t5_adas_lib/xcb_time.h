#pragma once

#include <iostream>
#include <memory>
#include <string>
#ifdef __linux__
#include <sys/time.h>
#elif _WIN32
#include "windows.h"
#else
#include <ctime>
#endif // __linux__

class TimeInterval
{
public:
	TimeInterval(const std::string& d) : detail(d)
	{
		init();
	}

	TimeInterval()
	{
		init();
	}

	~TimeInterval()
	{
#ifdef __linux__
		gettimeofday(&end, NULL);
//		std::cout << detail 
//			<< 1000.0 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000.0
//			<< " ms" << std::endl;
		printf("%s%lf ms\n", detail.c_str(), 
			1000.0 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000.0);
#elif _WIN32
		QueryPerformanceCounter(&nEndTime);
		std::cout << detail
			<< (nEndTime.QuadPart-m_nBeginTime.QuadPart)*1000.0/m_nFreq.QuadPart
			<< " ms" << std::endl;
#else
		end = clock();
		std::cout << detail 
			<< (double)(end - start) << " ms" << std::endl;
#endif // __linux__
	}

	long long getCurrentTime()
	{
#ifdef __linux__
		gettimeofday(&end, NULL);
		return (long long)(1000.0 * end.tv_sec + end.tv_usec / 1000.0 + 0.5);
#elif _WIN32
		QueryPerformanceCounter(&nEndTime);
		return (long long)(nEndTime.QuadPart*1000.0 / m_nFreq.QuadPart + 0.5);
#else
		end = clock();
		return (long long)(end + 0.5);
#endif // __linux__
	}
protected:
	void init() {
#ifdef __linux__
		gettimeofday(&start, NULL);
#elif _WIN32
		QueryPerformanceFrequency(&m_nFreq); 
		QueryPerformanceCounter(&m_nBeginTime);
#else
		start = clock();
#endif // __linux__
	}
private:
	std::string detail;
#ifdef __linux__
	timeval start, end;
#elif _WIN32
	LARGE_INTEGER m_nFreq;  
	LARGE_INTEGER m_nBeginTime;  
	LARGE_INTEGER nEndTime;
#else
	clock_t start, end;
#endif // __linux__
};

#define XCB_TIME_START(d)   {std::shared_ptr<TimeInterval> time_interval_scope_begin = std::make_shared<TimeInterval>(d);
#define XCB_TIME_END }

