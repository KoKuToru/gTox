ALTER TABLE log ADD COLUMN filenumber INTEGER;
ALTER TABLE log ADD COLUMN filesize INTEGER;

UPDATE log SET type = 
CASE 
	WHEN type = 1 AND sendtime IS NOT NULL THEN 1
	WHEN type = 1 AND sendtime IS NULL THEN 2
	WHEN type = 2 AND sendtime IS NOT NULL THEN 3
	WHEN type = 2 AND sendtime IS NULL THEN 4
END;
