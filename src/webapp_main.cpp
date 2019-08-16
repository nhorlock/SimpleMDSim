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

int main() {
	/* Overly simple hello world app */
	uWS::App()
        .get("/*", [](auto *res, auto *req) {
	        res->end("Hello world!");
	    })
        // .get("/simple", [](auto *res, auto *req){
        //     std::cou
        // })
        .listen(3000, [](auto *token) {
	        if (token) {
		        std::cout << "Listening on port " << 3000 << std::endl;
	        }
	    })
        .run();

	std::cout << "Failed to listen on port 3000" << std::endl;
}