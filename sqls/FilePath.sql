CREATE TABLE FilePath (
    file_id INT AUTO_INCREMENT PRIMARY KEY,
    model_id INT NOT NULL,
    file_path VARCHAR(1024) NOT NULL,
    last_updated TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (model_id) REFERENCES ModelBluePrint(model_id) ON DELETE CASCADE
);