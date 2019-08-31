#include <catch2/catch.hpp>
#include <sqlite_modern_cpp.h>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <chrono>
#include "symbollist.h"

using namespace fmt::literals;
using string = std::string;

SymbolStore s;

TEST_CASE("symbol list for ticker", "[refdata]")
{
    std::string res;
    res = s.getSymbolDataForTickerAsJSON("ibm");
    CHECK_NOTHROW(json::parse(res));
    CHECK(json::parse(res)[0].at("symbolID") == 127);
}


TEST_CASE("requesting 10 rows, ibm, now", "[tickdata]")
{
    std::string res;
    json results;

    try
    {
        res = s.getStockPricesForSymbolAsJSON("ibm", 10);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    REQUIRE_NOTHROW(results = json::parse(res));
    CHECK(results[0].at("symbol") == "ibm");
    CHECK(results.size() == 10);
}

TEST_CASE("request 1 row at time offset 0", "[tickdata]")
{
    std::string res;
    json results;
    res = s.getStockPricesForSymbolAsJSON("ibm", 1, 0);
    REQUIRE_NOTHROW(results = json::parse(res));
    CHECK(results[0].at("symbol") == "ibm");
    // std::cout << results.dump(1) << "\n";
    CHECK(results.size() == 1);
    CHECK(results[0].at("timeStamp") == "00:00:00");
    CHECK(results[0].at("periodNumber") == 1);
}

TEST_CASE("request 1 row at time offset 1", "[tickdata]")
{
    std::string res;
    json results;
    res = s.getStockPricesForSymbolAsJSON("ibm", 1, 1);
    REQUIRE_NOTHROW(results = json::parse(res));
    CHECK(results[0].at("symbol") == "ibm");
    //        std::cout << results.dump(1) << "\n";
    CHECK(results.size() == 1);
    CHECK(results[0].at("timeStamp") == "00:00:01");
    CHECK(results[0].at("periodNumber") == 1);
}
TEST_CASE("request 1 row at time offset 20", "[tickdata]")
{
    std::string res;
    json results;
    res = s.getStockPricesForSymbolAsJSON("ibm", 1, 20);
    REQUIRE_NOTHROW(results = json::parse(res));
    CHECK(results[0].at("symbol") == "ibm");
    //        std::cout << results.dump(1) << "\n";
    CHECK(results.size() == 1);
    CHECK(results[0].at("timeStamp") == "00:00:20");
    CHECK(results[0].at("periodNumber") == 2);
}
TEST_CASE("request 2 rows at time offset 0 - should wrap backwards", "[tickdata]")
{
    std::string res;
    json results;
    res = s.getStockPricesForSymbolAsJSON("ibm", 2, 0);
    REQUIRE_NOTHROW(results = json::parse(res));
    CHECK(results[0].at("symbol") == "ibm");
    // std::cout << results.dump(1) << "\n";
    CHECK(results.size() == 2);
    CHECK(results[0].at("timeStamp") == "23:59:59");
    CHECK(results[0].at("periodNumber") == 4320);
    CHECK(results[0].at("price") == results[1].at("price"));
    CHECK(results[1].at("timeStamp") == "00:00:00");
    CHECK(results[1].at("periodNumber") == 1);
}
TEST_CASE("request 10 rows at time offset 4 (window -5 - +5 expected)", "[tickdata]")
{
    std::string res;
    json results;
    res = s.getStockPricesForSymbolAsJSON("ibm", 10, 4);
    REQUIRE_NOTHROW(results = json::parse(res));
    CHECK(results[0].at("symbol") == "ibm");
    std::cout << results.dump(1) << "\n";
    CHECK(results.size() == 10);
    CHECK(results[0].at("timeStamp") == "23:59:55");
    CHECK(results[0].at("periodNumber") == 4320);
    CHECK(results[9].at("timeStamp") == "00:00:04");
    CHECK(results[9].at("periodNumber") == 1);
    auto j= results.size();
    j--;
    for(auto i=0; i <results.size();i++){
        CHECK(results[i].at("price") == results[j-i].at("price"));
    }
}

TEST_CASE("time offset 21609 (end of first window+10) count 20", "[tickdata]")
{
    std::string res;
    json results;
    res = s.getStockPricesForSymbolAsJSON("ibm", 20, 21609);
    REQUIRE_NOTHROW(results = json::parse(res));
    CHECK(results[0].at("symbol") == "ibm");
    // std::cout << results.dump(1) << "\n";
    CHECK(results.size() == 20);
    CHECK(results[0].at("timeStamp") == "05:59:50");
    CHECK(results[0].at("periodNumber") == 1080);
    CHECK(results[19].at("timeStamp") == "06:00:09");
    CHECK(results[19].at("periodNumber") == 1081);

    auto j= results.size();
    j--;
    for(auto i=0; i <results.size();i++){
        CHECK(results[i].at("price") == results[j-i].at("price"));
    }
}

TEST_CASE("stamp to offset", "[timestamp]")
{
    REQUIRE_NOTHROW(s.offsetFromTimeStamp("11:11:11"));
    CHECK_THROWS(s.offsetFromTimeStamp("24:11:11"));
    CHECK_THROWS(s.offsetFromTimeStamp("11:11:60")); // 61st second is only valid as the last second of the day
    CHECK_NOTHROW(s.offsetFromTimeStamp("23:59:60"));
    CHECK(s.offsetFromTimeStamp("00:00:00") == 0);
    CHECK(s.offsetFromTimeStamp("00:00:01") == 1);
    CHECK(s.offsetFromTimeStamp("00:01:00") == 60);
    CHECK(s.offsetFromTimeStamp("00:10:00") == 600);
    CHECK(s.offsetFromTimeStamp("01:00:00") == 3600);
    CHECK(s.offsetFromTimeStamp("10:00:00") == 36000);
    CHECK(s.offsetFromTimeStamp("23:59:59") == 23 * 60 * 60 + (59 * 60) + 59);
}
TEST_CASE("timestamp from offset", "[timestamp]")
{
    CHECK(s.timestampFromOffset(3600) == "01:00:00");
    CHECK(s.timestampFromOffset(3601) == "01:00:01");
    CHECK(s.timestampFromOffset(3660) == "01:01:00");
    CHECK(s.timestampFromOffset(3661) == "01:01:01");
    CHECK(s.timestampFromOffset(3659) == "01:00:59");
}
TEST_CASE("given period return offset of period start", "[timestamp]")
{
    REQUIRE_NOTHROW(s.offsetFromPeriod(1));
    CHECK_THROWS(s.offsetFromPeriod(0));    //must be more than 1
    CHECK_THROWS(s.offsetFromPeriod(6000)); //must be less than 21600*4/20
    CHECK(s.offsetFromPeriod(1) == 0);
    CHECK(s.offsetFromPeriod(2) == 20);
    CHECK(s.offsetFromPeriod(187) == 20 * 186);
    CHECK(s.offsetFromPeriod(2500) == 49980);
    CHECK(s.offsetFromPeriod(2501) == 50000);
}
TEST_CASE("given offset return period in which it exists", "[timestamp]")
{
    CHECK(s.periodContainingOffset(0) == 1);
    CHECK(s.periodContainingOffset(1) == 1);
    CHECK(s.periodContainingOffset(20) == 2);
    CHECK(s.periodContainingOffset(2000) == 101);
    CHECK(s.periodContainingOffset(2001) == 101);
}
TEST_CASE("Given offset return offset of period start", "[timestamp]")
{
    CHECK(s.periodStartFromOffset(0) == 0);
    CHECK(s.periodStartFromOffset(1) == 0);
    CHECK(s.periodStartFromOffset(19) == 0);
    CHECK(s.periodStartFromOffset(20) == 20);
    CHECK(s.periodStartFromOffset(27) == 20);
    CHECK(s.periodStartFromOffset(40) == 40);
    CHECK(s.periodStartFromOffset(25200) == 25200);
    CHECK(s.periodStartFromOffset(25210) == 25200);
}
TEST_CASE("iterate over 24 hours ensure consecutive periods", "[timestamp]")
{
    for (auto i = 0, j = 1; i < 4 * 21600; i += 20, j++)
    {
        REQUIRE(s.periodContainingOffset(i) == j);
        REQUIRE(i == s.offsetFromTimeStamp(s.timestampFromOffset(i)));
    }
}

TEST_CASE("Period 2", "[statsdata]")
{
    std::string res;
    json results;
    res = s.getStatisticalPricesForSymbolAndPeriodAsJSON("ibm", 2);
    REQUIRE_NOTHROW(results = json::parse(res));
    CHECK(results.size() == 1);
    CHECK(results[0].at("symbol") == "ibm");
    std::cout << results.dump(1) << "\n";
    CHECK(results[0].at("periodStartTime") == "00:00:20");
    CHECK(results[0].at("periodEndTime") == "00:00:39");
    CHECK(results[0].at("periodNumber") == 2);
}
TEST_CASE("Period 4000", "[statsdata]")
{
    std::string res;
    json results;
    res = s.getStatisticalPricesForSymbolAndPeriodAsJSON("ibm", 4000);
    REQUIRE_NOTHROW(results = json::parse(res));
    CHECK(results.size() == 1);
    CHECK(results[0].at("symbol") == "ibm");
    std::cout << results.dump(1) << "\n";
    CHECK(results[0].at("periodStartTime") == "22:13:00");
    CHECK(results[0].at("periodEndTime") == "22:13:19");
    CHECK(results[0].at("periodNumber") == 4000);
}
TEST_CASE("Period 1 == Period 1081", "[statsdata]")
{
    std::string res;
    json result1;
    json result2;
    res = s.getStatisticalPricesForSymbolAndPeriodAsJSON("ibm", 1);
    REQUIRE_NOTHROW(result1 = json::parse(res));
    CHECK(result1.size() == 1);
    CHECK(result1[0].at("symbol") == "ibm");
    std::cout << result1.dump(1) << "\n";
    res = s.getStatisticalPricesForSymbolAndPeriodAsJSON("ibm", 1081);
    REQUIRE_NOTHROW(result2 = json::parse(res));
    CHECK(result2.size() == 1);
    CHECK(result2[0].at("symbol") == "ibm");
    std::cout << result2.dump(1) << "\n";

    CHECK(result1[0].at("periodStartTime") != result2[0].at("periodStartTime"));
    CHECK(result1[0].at("periodEndTime") != result2[0].at("periodEndTime"));
    CHECK(result1[0].at("periodNumber") != result2[0].at("periodNumber"));
    CHECK(result1[0].at("minPrice") == result2[0].at("minPrice"));
    CHECK(result1[0].at("maxPrice") == result2[0].at("maxPrice"));
    CHECK(result1[0].at("openingPrice") == result2[0].at("closingPrice"));
    CHECK(result1[0].at("closingPrice") == result2[0].at("openingPrice"));
}
TEST_CASE("Period 1 == Period 2161", "[statsdata]")
{
    std::string res;
    json result1;
    json result2;
    res = s.getStatisticalPricesForSymbolAndPeriodAsJSON("ibm", 1);
    REQUIRE_NOTHROW(result1 = json::parse(res));
    CHECK(result1.size() == 1);
    CHECK(result1[0].at("symbol") == "ibm");
    std::cout << result1.dump(1) << "\n";
    res = s.getStatisticalPricesForSymbolAndPeriodAsJSON("ibm", 2161);
    REQUIRE_NOTHROW(result2 = json::parse(res));
    CHECK(result2.size() == 1);
    CHECK(result2[0].at("symbol") == "ibm");
    std::cout << result2.dump(1) << "\n";

    CHECK(result1[0].at("periodStartTime") != result2[0].at("periodStartTime"));
    CHECK(result1[0].at("periodEndTime") != result2[0].at("periodEndTime"));
    CHECK(result1[0].at("periodNumber") != result2[0].at("periodNumber"));
    CHECK(result1[0].at("minPrice") == result2[0].at("minPrice"));
    CHECK(result1[0].at("maxPrice") == result2[0].at("maxPrice"));
    CHECK(result1[0].at("openingPrice") == result2[0].at("openingPrice"));
    CHECK(result1[0].at("closingPrice") == result2[0].at("closingPrice"));
}
TEST_CASE("Period rollover testing", "[statsdata]")
{
    std::string res;
    json p1, p2;

    res = s.getStatisticalPricesForSymbolAndPeriodAsJSON("ibm", 1080);
    REQUIRE_NOTHROW(p1 = json::parse(res));
    res = s.getStatisticalPricesForSymbolAndPeriodAsJSON("ibm", 1081);
    REQUIRE_NOTHROW(p2 = json::parse(res));
    CHECK(p1.size() == 1);
    CHECK(p2.size() == 1);
    CHECK(p1[0].at("symbol") == "ibm");
    CHECK(p2[0].at("symbol") == "ibm");
    std::cout << p1.dump(1) << "\n";
    std::cout << p2.dump(1) << "\n";
    CHECK(p1[0].at("periodStartTime") == "05:59:40");
    CHECK(p1[0].at("periodEndTime") == "05:59:59");
    CHECK(p2[0].at("periodStartTime") == "06:00:00");
    CHECK(p2[0].at("periodEndTime") == "06:00:19");
    CHECK(p1[0].at("periodNumber") == 1080);
    CHECK(p2[0].at("periodNumber") == 1081);
}

// TEST_CASE("Period 3820", "[statsdata]")
// {
//     std::string res;
//     json results;
//     res = s.getStatisticalPricesForSymbolAndPeriodAsJSON("ibm", 3820);
//     REQUIRE_NOTHROW(results = json::parse(res));
//     CHECK(results.size() == 1);
//     CHECK(results[0].at("symbol") == "ibm");
//     std::cout << results.dump(1) << "\n";
//     CHECK(results[0].at("periodStartTime") == "00:00:20");
//     CHECK(results[0].at("periodEndTime") == "00:00:39");
//     CHECK(results[0].at("periodNumber") == 3820);
// }
// TEST_CASE("Period 4180", "[statsdata]")
// {
//     std::string res;
//     json results;
//     res = s.getStatisticalPricesForSymbolAndPeriodAsJSON("ibm", 4180);
//     REQUIRE_NOTHROW(results = json::parse(res));
//     CHECK(results.size() == 1);
//     CHECK(results[0].at("symbol") == "ibm");
//     std::cout << results.dump(1) << "\n";
//     CHECK(results[0].at("periodStartTime") == "00:00:20");
//     CHECK(results[0].at("periodEndTime") == "00:00:39");
//     CHECK(results[0].at("periodNumber") == 4180);
// }
