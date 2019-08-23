/**
 * @file shortloop.cpp
 * @author Neil Horlock (nhorlock@gmail.com)
 * @brief shortloop mode for the simple MD server
 * @description Provides the short loop implementation for the MD server
 * In this mode the data is entirely faked and guarentted to repeat every num_samples requests
 * Data is pseudo random and seeded from a hash of the symbol.
 * @version 0.1
 * @date 2019-08-21
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#include "shortloop.h"

std::unordered_map<std::string_view, shortloop::SimpleData> shortloop::waveMap;

int shortloop::randomPrice()
{
	return(std::rand()%100000);
}

int shortloop::randomPrice(int base, int vol, int sen)
{
	int change = std::rand();
	change = change%(int)((double)base*((double)vol/100));
	int direction = ((std::rand()%100)>sen)?1:-1;

	return base+(change*direction);
}

int shortloop::randomVolatility()
{
	return((std::rand()%(max_volatility-1))+1);
}

int shortloop::randomSentiment()
{
	return(std::rand()%100);
}

int shortloop::getSimpleDataForTicker(std::string_view& ticker)
{
	int retval=0;
	auto found = waveMap.find(ticker);
	SimpleData s(num_samples);

	if(found == waveMap.end())
	{
		auto seed = std::hash<std::string_view>();
		std::srand(seed(ticker));
		auto base=randomPrice();
		auto vol=randomVolatility();
		auto sentiment=randomSentiment();
		s.data[0]=base;
		for(int i=1; i<num_samples;i++)
		{
			s.data[i]=randomPrice(s.data[i-1],vol,sentiment);
		}
		waveMap.insert( {ticker, s} );
	}
	else
	{	
		s = found->second;
	}
	retval = s.data[s.count];
	s.count = (s.count+1)%num_samples;
	waveMap.insert_or_assign(ticker,s);
	return(retval);
}
