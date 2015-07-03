CREATE TABLE config(
    name TEXT,
    value BLOB
);

CREATE TABLE toxcore(
    id INTEGER PRIMARY KEY,
    savetime TEXT,
    state INTEGER
);

CREATE TABLE log(
    friendaddr BLOB,
    sendtime TEXT,
    recvtime TEXT,
    type INTEGER,
    message BLOB
    receipt BOOLEAN
);

CREATE TABLE delcontact(
    friendaddr BLOB,
    deletetime INTEGER
);

INSERT INTO config(name, value) VALUES('version', 1);
