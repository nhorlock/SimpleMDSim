-- SQLite
CREATE INDEX idx1 on tickdata(tradeTimeOffset, symbolID);
select count(*) from tickdata;