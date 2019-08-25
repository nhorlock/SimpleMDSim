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
    res = s.getStockPricesForSymbolAsJSON("ibm", 10);
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
TEST_CASE("request 20 rows at time offset 20", "[tickdata]")
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
    CHECK(results[1].at("timeStamp") == "00:00:00");
    CHECK(results[1].at("periodNumber") == 1);
}
TEST_CASE("request 10 rows at time offset 5 (window -5 - +5 expected)", "[tickdata]")
{
    std::string res;
    json results;
    res = s.getStockPricesForSymbolAsJSON("ibm", 10, 5);
    REQUIRE_NOTHROW(results = json::parse(res));
    CHECK(results[0].at("symbol") == "ibm");
    // std::cout << results.dump(1) << "\n";
    CHECK(results.size() == 10);
    CHECK(results[0].at("timeStamp") == "23:59:56");
    CHECK(results[0].at("periodNumber") == 4320);
    CHECK(results[9].at("timeStamp") == "00:00:05");
    CHECK(results[9].at("periodNumber") == 1);
}

TEST_CASE("time offset 21610 (end of first window+10) count 20", "[tickdata]")
{
    std::string res;
    json results;
    res = s.getStockPricesForSymbolAsJSON("ibm", 20, 21610);
    REQUIRE_NOTHROW(results = json::parse(res));
    CHECK(results[0].at("symbol") == "ibm");
    // std::cout << results.dump(1) << "\n";
    CHECK(results.size() == 20);
    CHECK(results[0].at("timeStamp") == "05:59:51");
    CHECK(results[0].at("periodNumber") == 1080);
    CHECK(results[19].at("timeStamp") == "06:00:10");
    CHECK(results[19].at("periodNumber") == 1081);
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
