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
    // a simple struct to model a person
    struct symbol {
        int symbolID;
        std::string symbol;
        std::string companyName;
    };
    void to_json(json& j, const symbol& s) {
        j = json{{"companyName", s.companyName}, {"symbol", s.symbol}, {"symbolID", s.symbolID}};
    }
    void from_json(const json& j, symbol& s) {
        j.at("companyName").get_to(s.companyName);
        j.at("symbol").get_to(s.symbol);
        j.at("symbolID").get_to(s.symbolID);
    }
}
class SymbolStore
{
private:
    sqlite::database db;
public:
    SymbolStore():db("tickdata.db")
    {}
       // Same database as above
//        sqlite::database db("tickdata.db");
 // symbols are output as a JSON array as follows
    //  {
    //     "SymbolID": 138,
    //     "Symbol": "a",
    //     "companyName": "Agilent Technologies Inc"
    // },
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
};
