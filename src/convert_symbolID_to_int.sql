-- SQLite
BEGIN TRANSACTION;
SELECT * FROM symbolmap WHERE symbolID="127";
ALTER TABLE symbolmap RENAME TO _symbolmap;
CREATE TABLE symbolmap
( 
    symbolID INTEGER,
    symbol text,
    CompanyName text
);

INSERT INTO symbolmap (symbolID, symbol, CompanyName) 
    SELECT CAST(symbolID as INTEGER), symbol, CompanyName
    FROM _symbolmap;

SELECT * FROM symbolmap WHERE symbolID="127";

DROP TABLE _symbolmap;
COMMIT;