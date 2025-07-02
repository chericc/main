#include <stdio.h>
#include <sqlite3.h>


    
// Callback function for sqlite3_exec
static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    for (int i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

int main() {
    sqlite3 *db;
    char *err_msg = 0;
    int rc;
    
    // Open or create a database file
    rc = sqlite3_open("demo.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }
    
    // Create a table
    const char *create_table_sql = 
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "email TEXT NOT NULL UNIQUE,"
        "age INTEGER);";
    
    rc = sqlite3_exec(db, create_table_sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    }
    
    // Insert some data
    const char *insert_sql = 
        "INSERT INTO users (name, email, age) VALUES "
        "('John Doe', 'john@example.com', 30),"
        "('Jane Smith', 'jane@example.com', 25),"
        "('Bob Johnson', 'bob@example.com', 40);";
    
    rc = sqlite3_exec(db, insert_sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
    
    // Query the data
    const char *select_sql = "SELECT * FROM users;";
    printf("All users:\n");
    rc = sqlite3_exec(db, select_sql, callback, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
    
    // Close the database
    sqlite3_close(db);
    
    printf("Database operations completed successfully.\n");
    return 0;
}
