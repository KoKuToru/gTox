CREATE TABLE `file` (
	`id`	INTEGER NOT NULL UNIQUE,
	`is_recv`	INTEGER,
	`friend_addr`	BLOB,
	`file_nr`	INTEGER,
	`file_id`	BLOB,
	`file_kind`	INTEGER,
	`file_path`	BLOB,
	`file_size`	INTEGER,
	`status`	INTEGER,
	PRIMARY KEY(id)
);
