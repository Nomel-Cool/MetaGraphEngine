-- QtGraphEngineKit database schema
-- Run with: mysql -u root -p --default-character-set=utf8mb4 < sqls/schema.sql

CREATE DATABASE IF NOT EXISTS meta_graph_model
  CHARACTER SET utf8mb4
  COLLATE utf8mb4_0900_ai_ci;

USE meta_graph_model;

CREATE TABLE IF NOT EXISTS ModelBluePrint (
  model_id INT NOT NULL AUTO_INCREMENT,
  name VARCHAR(255) NOT NULL,
  PRIMARY KEY (model_id),
  UNIQUE KEY name (name)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS FilePath (
  file_id INT NOT NULL AUTO_INCREMENT,
  model_id INT NOT NULL,
  file_path VARCHAR(1024) NOT NULL,
  last_updated TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (file_id),
  UNIQUE KEY model_id (model_id),
  CONSTRAINT filepath_ibfk_1 FOREIGN KEY (model_id)
    REFERENCES ModelBluePrint (model_id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS Automata (
  automata_id INT NOT NULL AUTO_INCREMENT,
  model_id INT NOT NULL,
  init_status TEXT NOT NULL,
  transfer_func VARCHAR(255) NOT NULL,
  current_input TEXT NOT NULL,
  current_status TEXT NOT NULL,
  terminate_status TEXT NOT NULL,
  checksum CHAR(64) NOT NULL,
  PRIMARY KEY (automata_id),
  KEY model_id (model_id),
  CONSTRAINT automata_ibfk_1 FOREIGN KEY (model_id)
    REFERENCES ModelBluePrint (model_id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;