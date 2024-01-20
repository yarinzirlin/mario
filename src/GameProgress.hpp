#include <sqlite3.h>
#include <string>

class GameProgress {
private:
    sqlite3* db;
    const char* dbFile = "game_progress.db";

    // Initialize the database
    void initDB() {
        if (sqlite3_open(dbFile, &db) == SQLITE_OK) {
            const char* createTableQuery = "CREATE TABLE IF NOT EXISTS PlayerProgress ("
                                           "current_level INTEGER, "
                                           "current_health INTEGER);";
            sqlite3_exec(db, createTableQuery, 0, 0, 0);
        } else {
            // Handle database opening error
        }
    }

public:
    GameProgress() {
        initDB();
    }

    ~GameProgress() {
        sqlite3_close(db);
    }

    // Save the current game state (level and health)
    void saveProgress(int currentLevel, int currentHealth) {
        std::string sql = "INSERT OR REPLACE INTO PlayerProgress (rowid, current_level, current_health) VALUES (1, " 
                          + std::to_string(currentLevel) + ", " + std::to_string(currentHealth) + ");";
        sqlite3_exec(db, sql.c_str(), 0, 0, 0);
    }

    // Load the game state (level and health)
    void loadProgress(int& currentLevel, int& currentHealth) {
        std::string sql = "SELECT current_level, current_health FROM PlayerProgress WHERE rowid = 1;";
        sqlite3_stmt* stmt;

        // Default values if no progress is saved
        currentLevel = 0; 
        currentHealth = 3;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                currentLevel = sqlite3_column_int(stmt, 0);
                currentHealth = sqlite3_column_int(stmt, 1);
            }
            sqlite3_finalize(stmt);
        }
    }
};
