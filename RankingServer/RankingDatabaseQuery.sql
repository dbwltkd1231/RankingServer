CREATE DATABASE RankingDatabase;
GO

USE RankingDatabase;
GO

CREATE TABLE Score(
player_id VARCHAR(16) PRIMARY KEY,
score INT NOT NULL,
late_update DATETIME NOT NULL
);
GO

CREATE TABLE Ranking(
rank INT PRIMARY KEY,
player_id VARCHAR(16) NOT NULL,
FOREIGN KEY (player_id) REFERENCES Score(player_id) ON DELETE CASCADE
);
GO

SELECT
r.rank,
s.player_id,
s.score,
s.late_update
FROM Ranking r
INNER JOIN Score s ON r.player_id=s.player_id
ORDER BY r.rank;
GO

CREATE PROCEDURE InsertScores
AS
BEGIN
    DECLARE @i INT = 1;
    WHILE @i <= 100
    BEGIN
        DECLARE @player_id NVARCHAR(16) = CONCAT('player', @i);
        DECLARE @score INT = FLOOR(RAND() * 2000) + 500;
        DECLARE @random_date DATETIME = DATEADD(DAY, FLOOR(RAND() * 30), '2025-05-01 00:00:00');

        INSERT INTO Score (player_id, score, late_update)
        VALUES (@player_id, @score, @random_date);

        SET @i = @i + 1;
    END
END;
GO

EXEC InsertScores;
GO

CREATE PROCEDURE UpdateRanking
AS
BEGIN
    TRUNCATE TABLE Ranking;

    INSERT INTO Ranking (rank, player_id)
    SELECT ROW_NUMBER() OVER (ORDER BY score DESC) AS rank, player_id
    FROM Score;
END;
GO

EXEC UpdateRanking;
GO

