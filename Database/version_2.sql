BEGIN TRANSACTION;

--UPDATE LOG
ALTER TABLE log RENAME TO log_tmp;

CREATE TABLE log(
    id INTEGER PRIMARY KEY,
    friendaddr BLOB,
    sendtime TEXT,
    recvtime TEXT,
    type INTEGER,
    message BLOB,
    receipt INTEGER
);

INSERT INTO log(friendaddr, sendtime, recvtime, type, message, receipt)
SELECT friendaddr, sendtime, recvtime, type, message, 0
FROM log_tmp;

DROP TABLE log_tmp;

--UPDATE TOXCORE
ALTER TABLE toxcore RENAME TO toxcore_tmp;

CREATE TABLE toxcore(
    id INTEGER PRIMARY KEY,
    savetime TEXT,
    state INTEGER,
    runid INTEGER
);

INSERT INTO toxcore(id, savetime, state, runid)
SELECT id, savetime, state, 0
FROM toxcore_tmp;

--UPDATE CONFIG
DELETE FROM config WHERE name = 'version';
INSERT INTO config(name, value) VALUES('version', 2);
INSERT INTO config(name, value) VALUES('runid', 1);

COMMIT TRANSACTION;
