#ifndef SHORTLOOP_INCLUDED
/**
 * @file shortloop.h
 * @author Neil Horlock (nhorlock@gmail.com)
 * @brief header file for the shortloop dummy repeatable data mode
 * @version 0.1
 * @date 2019-08-21
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#pragma once
#define SHORTLOOP_INCLUDED

#include <unordered_map>
#include <functional>

constexpr int num_samples=10;
constexpr int max_volatility = 10;

namespace shortloop
{

    struct SimpleData{
    public:
        SimpleData(int size):data(size){};
        SimpleData(const SimpleData& o):count(o.count),data(o.data){};
        int count=0;
        std::vector<int> data;
    };

    extern std::unordered_map<std::string_view, SimpleData> waveMap;

	int randomPrice();
	int randomPrice(int base, int vol, int sen);
	int randomVolatility();
	int randomSentiment();
	int getSimpleDataForTicker(std::string_view& ticker);
}
#endif