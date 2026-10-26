-- Clear all QtGraphEngineKit tables safely.
USE meta_graph_model;
SET FOREIGN_KEY_CHECKS = 0;
TRUNCATE TABLE Automata;
TRUNCATE TABLE FilePath;
TRUNCATE TABLE ModelBluePrint;
SET FOREIGN_KEY_CHECKS = 1;
