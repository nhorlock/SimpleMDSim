/**
 * @file symbollist.h
 * @author Neil Horlock (nhorlock@gmail.com)
 * @brief return a json symbol list
 * @version 0.1
 * @date 2019-08-21
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#include <sqlite_modern_cpp.h>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <chrono>
#include "smdutils.h"

using namespace fmt::literals;
using json=nlohmann::json;

namespace ss {
    class NotFoundException: public std::exception
    {
        public:
        std::string_view ticker;
        NotFoundException(std::string_view& symbol):ticker(symbol){}
        virtual std::string what() throw()
        {
            return "Symbol '{}' does not exist."_format(ticker);
        } 
    };
    class BadTimeOffsetException: public std::exception
    {
        public:
        unsigned long timeOffset;
        BadTimeOffsetException(unsigned long time):timeOffset(time){}
        virtual std::string what() throw()
        {
            return "Time offset '{}' is not in the 24 hour range."_format(timeOffset);
        } 
    };
    class BadPeriodException: public std::exception
    {
        public:
        unsigned long period;
        BadPeriodException(unsigned long p):period(p){}
        virtual std::string what() throw()
        {
            return "Trading period '{}' is not a valid 20 second period in the 24 hour range."_format(period);
        } 
    };
    class BadTimeException: public std::exception
    {
        public:
        std::string_view timeStamp;
        BadTimeException(std::string& time):timeStamp(time){}
        virtual std::string what() throw()
        {
            return "Time Stamps must be HH:MM:SS '{}' is invalid."_format(timeStamp);
        } 
    };
    // a simple struct to model a person
    struct symbol {
        int symbolID;
        std::string symbol;
        std::string companyName;
    };
    void to_json(json& j, const symbol& s) {
        j = json{{"companyName", s.companyName}, {"symbol", s.symbol}, {"symbolID", s.symbolID}};
    };
    void from_json(const json& j, symbol& s) {
        j.at("companyName").get_to(s.companyName);
        j.at("symbol").get_to(s.symbol);
        j.at("symbolID").get_to(s.symbolID);
    };

    struct tick {
        std::string symbol;
        double price;
        std::string timeStamp;
        std::string companyName;
        unsigned long periodNumber;
    };
    void to_json(json& j, const tick& t) {
        j = json{{"symbol", t.symbol}, {"price", t.price}, {"timeStamp", t.timeStamp}, {"companyName", t.companyName}, {"periodNumber", t.periodNumber}};
    };
    void from_json(const json& j, tick& t) {
        j.at("companyName").get_to(t.companyName);
        j.at("symbol").get_to(t.symbol);
        j.at("price").get_to(t.price);
        j.at("timeStamp").get_to(t.timeStamp);
        j.at("periodNumber").get_to(t.periodNumber);
    };
    struct stats {
        unsigned long periodNumber;
        int symbolID;
        std::string symbol;
        double minPrice;
        double maxPrice;
        std::string periodStartTime;
        std::string periodEndTime;
        double openingPrice;
        double closingPrice;
    };
    void to_json(json& j, const stats& s) {
        j = json{
                 {"periodNumber", s.periodNumber},
                 {"symbol", s.symbol},
                 {"symbolID", s.symbolID},
                 {"minPrice", s.minPrice},
                 {"maxPrice", s.maxPrice},
                 {"periodStartTime", s.periodStartTime},
                 {"periodEndTime", s.periodEndTime},
                 {"openingPrice", s.openingPrice},
                 {"closingPrice", s.closingPrice}};
    };
    void from_json(const json& j, stats& s) {
        j.at("periodNumber").get_to(s.periodNumber);
        j.at("symbol").get_to(s.symbol);
        j.at("symbolID").get_to(s.symbolID);
        j.at("minPrice").get_to(s.minPrice);
        j.at("maxPrice").get_to(s.maxPrice);
        j.at("periodStartTime").get_to(s.periodStartTime);
        j.at("periodEndTime").get_to(s.periodEndTime);
        j.at("openingPrice").get_to(s.openingPrice);
        j.at("closingPrice").get_to(s.closingPrice);
    };
}
class SymbolStore
{
private:
    sqlite::database db;
    std::chrono::time_point<std::chrono::system_clock> last_midnight;
    static constexpr int six_hours=6*60*60;
    static constexpr int periods_per_six_hours=six_hours/20;
public:
    SymbolStore():db("tickdata.db")
    {
        using days = std::chrono::duration<int, std::ratio<86400>>;
        last_midnight = 
            std::chrono::time_point_cast<days>(std::chrono::system_clock::now());
    }

    auto getSymbolListAsJSON()
    {
        json symbols;
        db << "select symbolID, symbol, companyName from symbolmap;"
            >> [&](
             int symbolID,
             std::string symbol, 
             std::string companyName) {
                 ss::symbol s{symbolID, symbol, companyName};
                 symbols.push_back(s);
            };
        return symbols.dump();
    }
    auto getSymbolDataForTickerAsJSON(std::string_view ticker)
    {
        json symbols;
        auto q = "select symbolID, symbol, companyName from symbolmap where symbol=lower('{0}');"_format(ticker);
        db << q
            >> [&](
             int symbolID,
             std::string symbol, 
             std::string companyName) {
                 ss::symbol s{symbolID, symbol, companyName};
                 symbols.push_back(s);
            };
        if(symbols.is_null()) 
        {
            throw(new ss::NotFoundException(ticker));
        }
        return symbols.dump();
    }


    auto offsetFromTimeStamp(std::string timeStamp)
    {
        int hh,mm,ss;
        std::vector<std::string>  stringSplit = timeStamp | view::split(':');
        if ( stringSplit.size()!=3) throw(new ss::BadTimeException(timeStamp));
        try{
            hh=getnum<unsigned int>(stringSplit[0],0,23);
            mm=getnum<unsigned int>(stringSplit[1],0,59);
            ss=getnum<unsigned int>(stringSplit[2],0,60);//strictly speaking 60 is valid due to leap seconds.
        }
        catch(...)
        {
            throw(new ss::BadTimeException(timeStamp));
        }
        if(ss==60 && (hh!=23) && (mm != 59)){throw(new ss::BadTimeException(timeStamp));};
        return((hh*60*60)+(mm*60)+ss);
    }
    std::string timestampFromOffset(unsigned long secs_since_midnight)
    {
        int hh,mm,ss;
        hh=secs_since_midnight/3600;
        mm=((secs_since_midnight-(hh*3600))/60)%60;
        ss=secs_since_midnight%60;
        return("{0:02d}:{1:02d}:{2:02d}"_format(hh,mm,ss));
    }
    auto periodStartFromOffset(unsigned long secs_since_midnight)
    {
        if(secs_since_midnight>24*3600){throw(new ss::BadTimeOffsetException(secs_since_midnight));};
        return((secs_since_midnight/20)*20);
    }
    auto periodContainingOffset(unsigned long secs_since_midnight)
    {
        if(secs_since_midnight>24*3600){throw(new ss::BadTimeOffsetException(secs_since_midnight));};
        return(((secs_since_midnight/20)+1));
    }
    auto offsetFromPeriod(unsigned long periodNumber)
    {
        if(periodNumber < 1 || periodNumber > 24*3600/20){throw(new ss::BadPeriodException(periodNumber));};
        return((periodNumber-1)*20);
    }
/**
 * @brief Get the Stock Prices For Symbol As JSON
 * @description given a time and a number of rows we calculcate the window of data to return
 * as we only have 6 hours of data, we need to wrap around the data when we span window in a 24 hour period.
 * each row represents 1 second, there are 6*3600 (21600) rows per instrument.
 * thus if the time now is 06:00:35 and they pull back 50 rows we need to pull rows from 05:59:45 onwards
 * The selection algorithm for this is:
 * - seconds_now % 21600 = secs_this_window (number of seconds in the current window)
 * - seconds_last = num_rows- secs_this_window (the number we need from the previous window)
 * - period_multiplier = seconds_now/21600
 * - select from tickdata where tradetime>(21600-seconds_last) append results (actualPeriod=period*(periodMultiplier-1))
 * - select from tickdata where tradetime<(secs_this_window) append results (actualPeriod=period*(periodMultiplier))
 * @param ticker 
 * @param num_rows 
 * @return auto 
 */
    auto getStockPricesForSymbolAsJSON(std::string_view ticker, int num_rows, int seconds_since_midnight)
    {
        json ticks;

        auto start_window = seconds_since_midnight/six_hours;
        auto end_window{start_window};
        auto start_in_window = seconds_since_midnight%six_hours;
        auto end_in_window = (seconds_since_midnight-(num_rows-1))%six_hours;
        if((seconds_since_midnight-(num_rows-1))<start_window*six_hours)
        {
            end_window = start_window-1;
            if(end_window<0)
            {
                end_window=3;
                end_in_window += six_hours;
            }
        }
        auto end{end_in_window};
        auto ascending{true};
        auto period_offset=(start_window*periods_per_six_hours);
        auto period_offset_prev=(end_window*periods_per_six_hours);
        if(start_window%2 == 1){
            ascending=false;
        }
        if(start_window != end_window){
            auto qp = 
                "SELECT s.symbol, t.price, t.tradeTimeOffset, s.companyName, t.periodNumber "
                "FROM tickdata t "
                "INNER JOIN symbolmap s ON t.symbolID=s.symbolID "
                "WHERE s.symbol='{0}' AND t.tradeTimeOffset BETWEEN {1} AND {2} "
                "ORDER BY t.tradeTimeOffset {3};"_format(ticker, 
                                                         !ascending?end_in_window:0,
                                                         !ascending?six_hours-1:six_hours-(end_in_window+1),
                                                         !ascending?"ASC":"DESC");
            // std::cout << qp << "\n";
            db << qp
                >> [&](
                std::string symbol, 
                double price,
                unsigned long timeOffset,
                std::string companyName,
                unsigned long periodNumber) {
                    auto timestamp{timestampFromOffset(!ascending?(timeOffset+((end_window)*six_hours))
                                                                :((end_window+1)*six_hours)-(timeOffset+1))};
                    if(!ascending){ periodNumber += (end_window)*periods_per_six_hours;}
                    else { periodNumber = (end_window+1)*periods_per_six_hours-(periodNumber-1);};
                    ss::tick s{symbol, price, timestamp, companyName, periodNumber};
                    ticks.push_back(s);
                };
            end=0; // setup end for the next query
        }

        auto qp = 
            "SELECT s.symbol, t.price, t.tradeTimeOffset, s.companyName, t.periodNumber "
            "FROM tickdata t "
            "INNER JOIN symbolmap s ON t.symbolID=s.symbolID "
            "WHERE s.symbol='{0}' AND t.tradeTimeOffset BETWEEN {1} AND {2} "
            "ORDER BY t.tradeTimeOffset {3};"_format(ticker, 
                                                     ascending?end:six_hours-(start_in_window+1),
                                                     ascending?start_in_window:six_hours-(end+1),
                                                     ascending?"ASC":"DESC");
        std::cout << qp << "\n";
        db << qp
            >> [&](
            std::string symbol, 
            double price,
            unsigned long timeOffset,
            std::string companyName,
            unsigned long periodNumber) {
                // auto timestamp{timestampFromOffset(timeOffset+((start_window)*six_hours))};
                auto timestamp{timestampFromOffset(ascending?(timeOffset+((start_window)*six_hours))
                                                            :((start_window)*six_hours)+(six_hours-(timeOffset+1)))};
                if(ascending){ periodNumber += (start_window)*periods_per_six_hours;}
                else { periodNumber = (start_window*periods_per_six_hours)+(periods_per_six_hours-(periodNumber-1));};
                ss::tick s{symbol, price, timestamp, companyName, periodNumber};
                ticks.push_back(s);
            };


        if(ticks.is_null()) 
        {
            throw(new ss::NotFoundException(ticker));
        }
        return ticks.dump();
    }

    auto getStockPricesForSymbolAsJSON(std::string_view ticker, int num_rows)
    {
        using namespace std::chrono;
        auto seconds_now = time_point_cast<seconds>(system_clock::now());
        auto seconds_since_midnight = (seconds_now-time_point_cast<seconds>(last_midnight)).count();
        return getStockPricesForSymbolAsJSON(ticker, num_rows, seconds_since_midnight);
    }

    auto getStatisticalPricesForSymbolAndPeriodAsJSON(std::string_view ticker, unsigned long period)
    {
        auto windowedPeriod=((period-1)%(periods_per_six_hours)+1);
        json statsdata;
        auto ascending=((period-1)/periods_per_six_hours)%2==0?true:false;
        auto start=(windowedPeriod-1)*20;
        auto end=((windowedPeriod-1)*20)+19;
        auto q =
        R"(SELECT {0} as periodNumber, s.symbolID, s.symbol,
    MIN(price) AS minPrice, 
    MAX(price) AS maxPrice,
    (SELECT price FROM tickdata t WHERE t.symbolID=127 AND tradeTimeOffset={1}) AS openingPrice,
    (SELECT price FROM tickdata t WHERE t.symbolID=127 AND tradeTimeOffset={2}) AS closingPrice
FROM tickdata t
INNER JOIN symbolmap s ON t.symbolID=s.symbolID
WHERE periodNumber={4} AND s.symbol="{3}";)"_format(period, ascending?start:end, ascending?end:start, ticker, windowedPeriod);
        db << q
            >> [&](
             unsigned long periodNumber,
             int symbolID, 
             std::string symbol, 
             double minPrice,
             double maxPrice,
             double openingPrice,
             double closingPrice
             ) {
                ss::stats s{
                    periodNumber, symbolID, symbol, minPrice, maxPrice,
                    timestampFromOffset(offsetFromPeriod(period)),
                    timestampFromOffset(offsetFromPeriod(period)+19),
                    openingPrice, closingPrice};
                statsdata.push_back(s);
            };
        if(statsdata.is_null()) 
        {
            throw(new ss::NotFoundException(ticker));
        }
        return statsdata.dump();
    }
};
