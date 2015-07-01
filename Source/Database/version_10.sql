ALTER TABLE log ADD COLUMN fileid BLOB;
ALTER TABLE log ADD COLUMN status INTEGER;
ALTER TABLE log ADD COLUMN runid INTEGER;

CREATE TABLE log_column_rename (
	`id`	INTEGER,
	`friendaddr`	BLOB,
	`sendtime`	TEXT,
	`recvtime`	TEXT,
	`type`	INTEGER,
	`data`	BLOB,
	`receipt`	INTEGER,
	`filenumber`	INTEGER,
	`filesize`	INTEGER,
	`fileid`	BLOB,
	`status`	INTEGER,
	`runid`	INTEGER,
	PRIMARY KEY(id)
);
INSERT INTO log_column_rename SELECT `id`,`friendaddr`,`sendtime`,`recvtime`,`type`,`message`,`receipt`,`filenumber`,`filesize`,`fileid`,`status`,`runid` FROM `log`;
DROP TABLE `log`;
ALTER TABLE log_column_rename RENAME TO `log`;
