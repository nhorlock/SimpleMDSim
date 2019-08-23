-- SQLite
--SELECT *  FROM tickdata WHERE symbolID="127" AND id="5246349";
--select max(CAST(periodNumber AS INTEGER)) from tickdata;
-- select symbolID, tradeTime, time(tradeTime)-time('01:00:00'), price from tickdata LIMIT 5;
-- select symbolID, tradeTime, time(tradeTime), price from tickdata LIMIT 5;
-- select symbolID, tradeTime, strftime('%s',tradeTime)-strftime('%s','01:00:00'), price from tickdata LIMIT 5;
-- SELECT strftime('%s','01:00:05') - strftime('%s','01:00:00');
-- ALTER TABLE tickdata ADD COLUMN tradeTimeOffset Int;
BEGIN TRANSACTION;
SELECT * FROM tickdata WHERE symbolID="127" AND price="139.59" LIMIT 5;
ALTER TABLE tickdata RENAME TO _tickdata;
CREATE TABLE tickdata
( 
    tradeTimeOffset INTEGER,
    price REAL,
    symbolIndex INTEGER,
    symbolID INTEGER,
    periodNumber INTEGER
);

INSERT INTO tickdata (tradeTimeOffset, price, symbolIndex, symbolID, periodNumber) 
    SELECT strftime('%s', tradeTime)-strftime('%s','01:00:00'), price, symbolIndex, symbolID, periodNumber
    FROM _tickdata;

SELECT * FROM tickdata WHERE symbolID="127" AND price="139.59" LIMIT 5;

DROP TABLE _tickdata;
COMMIT;