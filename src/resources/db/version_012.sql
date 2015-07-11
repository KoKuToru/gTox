CREATE TABLE sqlitebrowser_rename_column_new_table (
	`id`	INTEGER,
	`friendaddr`	BLOB,
	`sendtime`	TEXT,
	`recvtime`	TEXT,
	`type`	INTEGER,
	`data`	BLOB,
	`receipt`	INTEGER,
	`fileid`	INTEGER,
	`status`	INTEGER,    
	PRIMARY KEY(id)
);

INSERT INTO sqlitebrowser_rename_column_new_table SELECT `id`,`friendaddr`,`sendtime`,`recvtime`,`type`,`data`,`receipt`,`fileid`, `status` FROM `log`;
DROP TABLE `log`;
ALTER TABLE `sqlitebrowser_rename_column_new_table` RENAME TO `log`;

DELETE FROM `log` WHERE `type`=5 OR `type`=6;

