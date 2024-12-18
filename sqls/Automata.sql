CREATE TABLE Automata (
    automata_id INT AUTO_INCREMENT PRIMARY KEY,
    model_id INT NOT NULL,
    init_status JSON NOT NULL,
    transfer_func VARCHAR(255) NOT NULL,
    current_input JSON NOT NULL,
    current_status JSON NOT NULL,
    terminate_status JSON NOT NULL,
    checksum CHAR(64) NOT NULL, -- Ð£ÑéºÍ£¨CRC£©
    FOREIGN KEY (model_id) REFERENCES ModelBluePrint(model_id) ON DELETE CASCADE
);