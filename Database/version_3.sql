CREATE TABLE bootstrap(
	id INTEGER PRIMARY KEY,
	active BOOLEAN,
	ip TEXT,
	port INTEGER,
	pub_key BLOB,
	name TEXT
);

INSERT INTO bootstrap(active, ip, port, pub_key, name) VALUES(1, 'kasahto.com',  33445, 'F80C126127006D039E953B6DEC0C9F372EC36D55C7A6E8E845E813EAFC5A6471', 'maurice');
INSERT INTO bootstrap(active, ip, port, pub_key, name) VALUES(1, 'urotukok.net', 33445, '0095FC11A624EEF1EED38B54A4BE3E7FF3527D367DC0ACD10AC8329C99319513', 'luca');

DELETE FROM config WHERE name = 'version';
INSERT INTO config(name, value) VALUES('version', 3);