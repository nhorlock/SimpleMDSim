#include <catch2/catch.hpp>
#include <chrono>
#include <string>
#include <iostream>
#include <utility>
#include <unordered_map>
#include <fmt/format.h>
#include <fmt/chrono.h>
#include <range/v3/all.hpp>

using namespace ranges;
using namespace fmt::literals;
using string = std::string;


std::unordered_map<std::string, std::string> parseFix (std::string message) {

    std::unordered_map<std::string, std::string> res={};
    std::vector<std::string> payload = message | view::split('\001');
    for(auto kv_pair : payload )
    {
        std::vector<std::string> kv = kv_pair | view::split('=');
        res[kv[0]]=kv[1];
    }
    return res;
};

#define nofmt(tag) fmt::arg( #tag , "{"#tag"}")
TEST_CASE("split simple") {
    std::string_view input{"arg=value&arg2=val=ue&arg3=234"};
    auto s=std::string(input);
    std::vector<std::string> tv_pairs=s | view::split('&');
    // TODO: error handling for badly formed FIX message
    auto split_kv = [](std::string p)->std::pair<std::string, std::string>{ auto found = p.find('='); return {p.substr(0, found), p.substr(found+1)};};
    CHECK(tv_pairs.size()==3);
    std::unordered_map<std::string, std::string> res={};
    for(auto p : tv_pairs){
        auto [k,v] = split_kv(p);
        res[k]=v;
    }
    CHECK(res.size()==3);
    CHECK(res["arg"] == "value");
    CHECK(res["arg2"] == "val=ue");
    CHECK(res["arg3"] == "234");
};