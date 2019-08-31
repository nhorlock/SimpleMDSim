# SimpleMDSim - Simple Market Data Simulator
A Simple Market Data Simulator for use with student projects.

## Requirements
* RESTful interface for Market Data
* Multiple output modes:
  * Simple waveform
  * Data replay (long loop)


# Design
The system runs as a REST service.
The RESTful service front-end exposes mutiple end-points, corresponding to the data being published
* random repeating
* feed

# Classes of data
There are 2 classes of data:
## Simple pseudo random
This is the most basic feed, a highly predictable, randomly generated wave of prices that cycles and repeats over a very short period. Price updates are triggered in response to a read allowing the use for testing against known results.

`/API/PseudoFeed/[ticker]`

Any string can be passed as the ticker.

### TL;DR technical info on the pseudo feed

The random numbers are seeded from the Hash of the ticker. The first values define the trend and volatility the remainder form the values. each call will return the next price.
* base - the first random number is the base price. All other prices are relative to this.
* volatility - the second random is a volatility, dictating how wildliy the price can swing from update to update. The max volatility is 10% random volatility will be between 0 and this.
* sentiment - the third random is the sentiment, this is how likely a price is to go up or down expressed as a percentage. A sentiment of 60% means that the price is likely to go down 60% of the time. This gives and artifical trend.
The next 100 values form the dataset, generated randomly within the preceding parameters.

## Simulated Feed

A long cycle simulation of real market data.
250+ symbols are available and can be queried using the GetSymbolList endpoint 
`/API/StockFeed/GetSymbolList`

For a known single symbol/ticker, the details can be retrieved using the `GetSymbolDetails/[TICKER]` endpoint. 
where `[TICKER]` is one of the valid symbols.
`http://feed2.conygre.com/API/StockFeed/GetSymbolDetails/ibm`

The main trade data "feed" is accessed via the `GetStockPricesForSymbol` endpoint. This endpoint takes a ticker, plus a couple of parameters.
* HowManyValues=[integer]
* WhatTime=[timestamp HH:MM:SS]

`/API/StockFeed/GetStockPricesForSymbol/ibm`

Will return 1 row (default) of IBM prices for the current second.

`/API/StockFeed/GetStockPricesForSymbol/ibm?HowManyValues=20`

Will return 20 rows up to and including the current second.

`/API/StockFeed/GetStockPricesForSymbol/ibm?HowManyValues=60&WhatTime=06:00:15`

Will return 60 seconds of prices up to and including 06:00:15

### Design and limitations of the feed
The price data was converted from real NYSE Market data at 15 minute intervals which have been reduced to one seconds intervals. 
there are 21600 rows for each unique instrument starting from 00:00:00 to 05:59:59. 

At 06:00:00 the data is reversed, this avoids a sawtooth-like reset of just replaying the data in loops.

A full 24 hour cycle is thus created from 4 segments, two forward and two reverse.

The data also provides the notion of a "period". The trading period can be used as if it were a trading day when computing statistical trends. The statistical data endpoint `API/StockFeed/GetOpenCloseMinMaxForSymbolAndPeriodNumber/ibm?PeriodNumber=325` provides accees to the computed High, Low, Open, and Close

`API/StockFeed/GetOpenCloseMinMaxForSymbolAndPeriodNumber/ibm?PeriodNumber=325`

Returns the statistical data for ibm  during the period number 325

Note: that the period for the current time is not provided, thus avoiding issues with wrap around data and algorithms accidentally predicting the future :-)


