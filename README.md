# SimpleMDSim - Simple Market Data Simulator
A Simple Market Data Simulator for use with student projects.

## Requirements
* RESTful interface for Market Data
* Multiple output modes:
  * Simple waveform
  * Short Canned loop (5 minutes)
  * Data replay (long loop)
* Feed format
  * JSON
  * request: Array of tickers
  * response: Array of data

## Design
The system runs as a web frontend, that reads from an in memory table.
The web front-end exposes mutiple end-points, corresponding to the data being published
* wave
* shortloop
* feed

Each end-point simply uses an in-memory table to retrieve the prices at that time. Price updates are asynchronous and produced by separate threads/actors.

The price Update actors are responsible for updating the in-memory table. They can be easily adapted to provide different classes of data. 

## Classes of data
There are 3 classes of data:
### Simple wave
This is the most basic feed, a highly predictable, randomly generated wave of prices that cycles and repeats over a very short period. Price updates are triggered in response to a read allowing the use for testing against known results.
### short loop
This mode generates more realisitc data but does so in a short 5 minute loop allowing repeatability of bugs during early development
### feed
A long replay limited only by the data provided. It will attempt to replay in step with the input data timestamps if available.


