/**
 * @file webapp_main.cpp
 * @author Neil Horlock (nhorlock@gmail.com)
 * @brief Main module for the SimpleMDServer web component
 * @version 0.1
 * @date 2019-08-16
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#include <iostream>
#include "App.h"
#include <fmt/format.h>
#include "shortloop.h"
#include "symbollist.h"
#include "range/v3/all.hpp"
#include <charconv>
#include "smdutils.h"
using namespace ranges;

SymbolStore s;

int main() {
	std::cout << "Starting app\n";
	/* stupidly simple webservice */
	uWS::App()
        .get("/API/PseudoFeed/:ticker", [](auto *res, auto *req) {
			auto ticker = req->getParameter(0);
			auto reply = shortloop::getSimpleDataForTicker(ticker);
			auto upd = fmt::format("{{{0}:{{last:{1}}}}}",req->getParameter(0),reply);
	        res->end(upd);
	    })
        .get("/API/StockFeed/GetSymbolList", [](auto *res, auto *req){
			try{
				res->writeHeader("Content-type", "text/json;");
				res->end(s.getSymbolListAsJSON());
			}
			catch(...)
			{
				res->writeStatus("400 Bad Request");
				res->writeHeader("Content-type", "text/html;");
				res->end("<H1>error</H1>");
			}
		})
        .get("/API/StockFeed/GetSymbolDetails/:ticker", [](auto *res, auto *req){
			try{
				auto resp_txt = s.getSymbolDataForTickerAsJSON(req->getParameter(0));
				res->writeHeader("Content-type", "text/json;");
				res->end(resp_txt);
			}
			catch(ss::NotFoundException* e)
			{
				res->writeStatus("404 Not Found");
				res->writeHeader("Content-type", "text/html;");
				res->end("<H1>error</H1><br/>{0}"_format(e->what()));
			}
		})
		.get("/API/StockFeed/GetOpenCloseMinMaxForSymbolAndPeriodNumber/:ticker", [](auto *res, auto *req){
			try{
				auto period=1;
				auto qp = std::string(req->getQuery());
				std::vector<std::string> queryParams = qp | view::split('&');
				for(auto sp : queryParams)
				{
					std::vector<std::string> kv = sp | view::split('=');
					auto key = kv[0]; auto value = kv[1];

					if(key == "PeriodNumber")
					{
						try
						{
							auto seconds_since_midnight = currentTimeOffset();							
							auto period_now=s.periodContainingOffset(seconds_since_midnight);
							period=getnum<unsigned int>(value,1,4320);
							if(period_now == period){
								throw(std::range_error("value not in expected range "));
							}
						}
						catch(...)
						{
							res->writeStatus("400 Bad Request");
							res->writeHeader("Content-type", "text/html;");
							res->end("<html><H1>Bad Request</H1><br/>'{0}' needs to be between 1 and 4320, and not current period - '{1}' was received.</html>"_format(key, value));
							return;
						}
					}
				}
				auto resp_txt=s.getStatisticalPricesForSymbolAndPeriodAsJSON(req->getParameter(0), period);
				res->writeHeader("Content-type", "text/json;");
				res->end(resp_txt);
			}
			catch(ss::NotFoundException* e)
			{
				res->writeStatus("404 Not Found");
				res->writeHeader("Content-type", "text/html;");
				res->end("<H1>error</H1><br/>{0}"_format(e->what()));
			}
		})
        .get("/API/StockFeed/GetStockPricesForSymbol/:ticker", [](auto *res, auto *req){
			try{
				auto num=1;
				unsigned int timeOffset=0;
				auto userTime=false;
				auto qp = std::string(req->getQuery());
				std::vector<std::string> queryParams = qp | view::split('&');
				for(auto sp : queryParams)
				{
					std::vector<std::string> kv = sp | view::split('=');
					auto key = kv[0]; auto value = kv[1];

					if(key == "HowManyValues")
					{
						try
						{
							num=getnum<unsigned int>(value,1,10000);
						}
						catch(...)
						{
							res->writeStatus("400 Bad Request");
							res->writeHeader("Content-type", "text/html;");
							res->end("<html><H1>Bad Request</H1><br/>A positive integer was expected for '{0}' and '{1}' was received.</html>"_format(key, value));
							return;
						}
					}
					if(key == "WhatTime")
					{
						try
						{
							userTime=true;
							timeOffset = s.offsetFromTimeStamp(value);
						}
						catch(...)
						{
							res->writeStatus("400 Bad Request");
							res->writeHeader("Content-type", "text/html;");
							res->end("<html><H1>Bad Request</H1><br/>A valid 24 hour timestamp HH:MM:SS' is expected for '{0}' and '{1}' was received.</html>"_format(key, value));
							return;
						}
					}
				}
				std::string resp_txt;
				if(!userTime)
				{
					resp_txt=s.getStockPricesForSymbolAsJSON(req->getParameter(0), num);
				}
				else
				{
					resp_txt=s.getStockPricesForSymbolAsJSON(req->getParameter(0), num, timeOffset);
				}
				res->writeHeader("Content-type", "text/json;");
				res->end(resp_txt);
			}
			catch(ss::NotFoundException* e)
			{
				res->writeStatus("404 Not Found");
				res->writeHeader("Content-type", "text/html;");
				res->end("<H1>error</H1><br/>{0}"_format(e->what()));
			}
		})
        .listen(3000, [](auto *token) {
	        if (token) {
		        std::cout << "Listening on port " << 3000 << "\n";
	        }
			else{
				std::cout << "Failed to listen on port 3000\n";
			}
	    })
        .run();

	std::cout << "Failed to listen on port 3000\n";
}