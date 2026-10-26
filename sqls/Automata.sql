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