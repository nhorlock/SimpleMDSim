#include <iostream>
#include <string>
#include <cstdlib>
#include <catch2/catch.hpp>

TEST_CASE("PRNG_IS_REPEATABLE") {
    //set up
    std::srand(42);
    std::vector<int> v={0,0,0,0,0,0,0,0,0,0};
    for (int i=0;i<v.size();i++)
    {
        v[i] = rand();
    }
    //test sections
    std::srand(42);
    SECTION("check PRNG is repeatable") {
        for(int i=0;i<v.size();i++){
            REQUIRE(v[i] == std::rand());
        }
    }
    //tear down
}
