/**
 * @file smdutils.h
 * @author Neil Horlock (nhorlock@gmail.com)
 * @brief shared functions for converting and stuff.
 * @version 0.1
 * @date 2019-08-25
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#pragma once
#include <iostream>
#include <charconv>
#include "range/v3/all.hpp"
using namespace ranges;

template<typename T = int>
auto getnum(std::string_view str)
{
    const auto fl = str.find_last_not_of(' ');	// Find end of data excluding trailing spaces
 
    if (fl == std::string_view::npos)	// If end of data not found, return no value
        return std::optional<T> {};
 
    const auto end = str.data() + fl + 1;	// End of data to be converted
    T num;
 
    return (std::from_chars(str.data() + str.find_first_not_of(' '), end, num).ptr == end) ? std::optional<T>{num} : std::optional<T> {};
}

template<typename T = int>
auto getnum(std::string_view str, T min, T max)
{
    if(auto val=getnum<unsigned int>(str); !val | val <min | val >max){throw(std::range_error("value not in expected range "));}
    else
    {
        return val.value();
    }
}