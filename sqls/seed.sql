-- Optional demo data for QtGraphEngineKit.
-- WARNING: this overwrites the automata rows and XML paths of the five demo
-- models if they already exist. Back up your database before running it.
-- Run with: mysql -u root -p --default-character-set=utf8mb4 < sqls/seed.sql

USE meta_graph_model;

-- ACell ---------------------------------------------------------------
INSERT INTO ModelBluePrint (name) VALUES ('ACell')
  ON DUPLICATE KEY UPDATE name = name;
SET @mid = (SELECT model_id FROM ModelBluePrint WHERE name = 'ACell');
DELETE FROM FilePath WHERE model_id = @mid;
INSERT INTO FilePath (model_id, file_path) VALUES (@mid, './resources/xmls/graphAutomata/ACell_Autogen.xml');
DELETE FROM Automata WHERE model_id = @mid;
INSERT INTO Automata (model_id, init_status, transfer_func, current_input, current_status, terminate_status, checksum)
  VALUES (@mid, '[]', 'CellAutomataFrame_0', '[]', '[]', '[]',
          SHA2('ACell', 256));

-- APointBlue ----------------------------------------------------------
INSERT INTO ModelBluePrint (name) VALUES ('APointBlue')
  ON DUPLICATE KEY UPDATE name = name;
SET @mid = (SELECT model_id FROM ModelBluePrint WHERE name = 'APointBlue');
DELETE FROM FilePath WHERE model_id = @mid;
INSERT INTO FilePath (model_id, file_path) VALUES (@mid, './resources/xmls/graphAutomata/APointBlue_Autogen.xml');
DELETE FROM Automata WHERE model_id = @mid;
INSERT INTO Automata (model_id, init_status, transfer_func, current_input, current_status, terminate_status, checksum)
  VALUES (@mid,
    '[{"x":1,"y":1,"z":1,"r":0.31,"g":0.5,"b":1.0,"a":1.0,"tag":17,"blockSize":1.0}]',
    'JustAPoint', '[]', '[]', '[]', SHA2('APointBlue', 256));

-- APointGreen ---------------------------------------------------------
INSERT INTO ModelBluePrint (name) VALUES ('APointGreen')
  ON DUPLICATE KEY UPDATE name = name;
SET @mid = (SELECT model_id FROM ModelBluePrint WHERE name = 'APointGreen');
DELETE FROM FilePath WHERE model_id = @mid;
INSERT INTO FilePath (model_id, file_path) VALUES (@mid, './resources/xmls/graphAutomata/APointGreen_Autogen.xml');
DELETE FROM Automata WHERE model_id = @mid;
INSERT INTO Automata (model_id, init_status, transfer_func, current_input, current_status, terminate_status, checksum)
  VALUES (@mid,
    '[{"x":2,"y":1,"z":1,"r":0.5,"g":1.0,"b":0.5,"a":1.0,"tag":18,"blockSize":1.0}]',
    'JustAPoint', '[]', '[]', '[]', SHA2('APointGreen', 256));

-- APointRed -----------------------------------------------------------
INSERT INTO ModelBluePrint (name) VALUES ('APointRed')
  ON DUPLICATE KEY UPDATE name = name;
SET @mid = (SELECT model_id FROM ModelBluePrint WHERE name = 'APointRed');
DELETE FROM FilePath WHERE model_id = @mid;
INSERT INTO FilePath (model_id, file_path) VALUES (@mid, './resources/xmls/graphAutomata/APointRed_Autogen.xml');
DELETE FROM Automata WHERE model_id = @mid;
INSERT INTO Automata (model_id, init_status, transfer_func, current_input, current_status, terminate_status, checksum)
  VALUES (@mid,
    '[{"x":1,"y":1,"z":1,"r":1.0,"g":0.5,"b":0.31,"a":1.0,"tag":16,"blockSize":1.0}]',
    'JustAPoint', '[]', '[]', '[]', SHA2('APointRed', 256));

-- DiagSegment ---------------------------------------------------------
INSERT INTO ModelBluePrint (name) VALUES ('DiagSegment')
  ON DUPLICATE KEY UPDATE name = name;
SET @mid = (SELECT model_id FROM ModelBluePrint WHERE name = 'DiagSegment');
DELETE FROM FilePath WHERE model_id = @mid;
INSERT INTO FilePath (model_id, file_path) VALUES (@mid, './resources/xmls/graphAutomata/DiagSegment_Autogen.xml');
DELETE FROM Automata WHERE model_id = @mid;
INSERT INTO Automata (model_id, init_status, transfer_func, current_input, current_status, terminate_status, checksum)
  VALUES (@mid,
    '[{"x":100,"y":100,"z":1,"r":1.0,"g":0.5,"b":0.31,"a":1.0,"tag":0,"blockSize":1.0}]',
    'BresenhamLine', '[{"tagHead":0}]', '[]',
    '[{"x":0.1,"y":0.1,"z":1}]', SHA2('DiagSegment', 256));

SELECT 'Seed complete. Models:' AS message;
SELECT m.name, f.file_path FROM ModelBluePrint m
  LEFT JOIN FilePath f ON f.model_id = m.model_id
  WHERE m.name IN ('ACell','APointBlue','APointGreen','APointRed','DiagSegment');
