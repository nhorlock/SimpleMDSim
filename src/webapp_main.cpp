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

SymbolStore s;

int main() {
	/* stupidly simple webservice */
	uWS::App()
        .get("/wave/:ticker", [](auto *res, auto *req) {
			auto ticker = req->getParameter(0);
			auto reply = shortloop::getSimpleDataForTicker(ticker);
			auto upd = fmt::format("{{{0}:{{last:{1}}}}}",req->getParameter(0),reply);
	        res->end(upd);
	    })
        .get("/API/StockFeed/GetSymbolList", [](auto *res, auto *req){
			res->writeHeader("Content-type", "text/json;");
			res->end(s.getSymbolListAsJSON());
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
			catch(...)
			{
				res->writeStatus("400 Bad Request");
				res->writeHeader("Content-type", "text/html;");
				res->end("<H1>error</H1>");
			}
		})
        .listen(3000, [](auto *token) {
	        if (token) {
		        std::cout << "Listening on port " << 3000 << "\n";
	        }
	    })
        .run();

	std::cout << "Failed to listen on port 3000\n";
}