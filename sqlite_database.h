#ifndef SQLITE_DATABASE_H
#define SQLITE_DATABASE_H
#include <stdio.h>
#include <sqlite3.h>
#include <string.h>

const char *db_name = "blockchain.db";

void query_database(sqlite3 *db, const char *sql_query)
{
    sqlite3_stmt *stmt; // Statement handle

    // Prepare the SQL query
    if (sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL) != SQLITE_OK)
    {
        printf("Failed to prepare query: %s\n", sqlite3_errmsg(db));
        return;
    }

    // Execute the query and iterate over the results
    printf("Results:\n");
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);                      // Get first column as an integer
        const unsigned char *name = sqlite3_column_text(stmt, 1);  // Get second column as text
        const unsigned char *email = sqlite3_column_text(stmt, 2); // Get third column as text

        printf("ID: %d, Name: %s, Email: %s\n", id, name, email);
    }

    // Finalize the statement to release resources
    sqlite3_finalize(stmt);
}

void fetch_block(char *height)
{
    char fetch_block_query[100] = "SELECT * FROM Blocks WHERE height=";
    const int query_len = strlen(fetch_block_query);
    strcat(fetch_block_query, height);
    strcat(fetch_block_query, ";");
    printf("\n%s\n", fetch_block_query);
}

void insert_block(char *height, char *timestamp, char *miner, char *merkle_root, char *nonce, char *block_hash, char *previous_block_hash)
{
    char insert_block_query[10000] = "INSERT INTO Blocks ('";
    strcat(insert_block_query, height);
    strcat(insert_block_query, "','");
    strcat(insert_block_query, timestamp);
    strcat(insert_block_query, "','");
    strcat(insert_block_query, miner);
    strcat(insert_block_query, "','");
    strcat(insert_block_query, merkle_root);
    strcat(insert_block_query, "','");
    strcat(insert_block_query, nonce);
    strcat(insert_block_query, "','");
    strcat(insert_block_query, block_hash);
    strcat(insert_block_query, "','");
    strcat(insert_block_query, previous_block_hash);
    strcat(insert_block_query, "');");
    printf("Insert block query: %s\n", insert_block_query);
}

void delete_transaction(char *id)
{
    char delete_transaction_query[512] = "DELETE FROM Transactions WHERE id='";
    strcat(delete_transaction_query, id);
    strcat(delete_transaction_query, "';");
    printf("Delete query: %s", delete_transaction_query);
}

void insert_transaction(char *id, char *block_id, char *sender_address, char *recepient_address, char *amount, char *raw_amount, char *timestamp, char *transaction_hash, char *signature, char *public_key)
{
    char insert_transaction_query[2048] = "INSERT INTO Transactions VALUES ('";
    strcat(insert_transaction_query, id);
    strcat(insert_transaction_query, "','");
    strcat(insert_transaction_query, block_id);
    strcat(insert_transaction_query, "','");
    strcat(insert_transaction_query, sender_address);
    strcat(insert_transaction_query, "','");
    strcat(insert_transaction_query, recepient_address);
    strcat(insert_transaction_query, "','");
    strcat(insert_transaction_query, amount);
    strcat(insert_transaction_query, "','");
    strcat(insert_transaction_query, raw_amount);
    strcat(insert_transaction_query, "','");
    strcat(insert_transaction_query, timestamp);
    strcat(insert_transaction_query, "','");
    strcat(insert_transaction_query, transaction_hash);
    strcat(insert_transaction_query, "','");
    strcat(insert_transaction_query, signature);
    strcat(insert_transaction_query, "','");
    strcat(insert_transaction_query, public_key);
    strcat(insert_transaction_query, "');");
    printf("\nInsert transaction query is: %s\n", insert_transaction_query);
}

void fetch_transactions(char *block_id)
{
    char fetch_transactions_query[100] = "SELECT * FROM Transactions WHERE block_id=";
    strcat(fetch_transactions_query, block_id);
    strcat(fetch_transactions_query, ";");
    printf("\n%s\n", fetch_transactions_query);
}

// int kain()
// {
//     sqlite3 *db; // SQLite database handle

//     const char *create_table_sql = "CREATE TABLE IF NOT EXISTS users ("
//                                    "id INTEGER PRIMARY KEY, "
//                                    "name TEXT NOT NULL, "
//                                    "email TEXT UNIQUE NOT NULL);";
//     const char *insert_sql = "INSERT INTO users (name, email) VALUES "
//                              "('Alice', 'alice@example.com'), "
//                              "('Bob', 'bob@example.com');";
//     const char *select_sql = "SELECT * FROM users;";

//     // Open the database
//     if (sqlite3_open(db_name, &db) != SQLITE_OK)
//     {
//         printf("Cannot open database: %s\n", sqlite3_errmsg(db));
//         return 1;
//     }

//     // Create table
//     if (sqlite3_exec(db, create_table_sql, 0, 0, 0) != SQLITE_OK)
//     {
//         printf("Failed to create table: %s\n", sqlite3_errmsg(db));
//     }

//     // Insert data
//     if (sqlite3_exec(db, insert_sql, 0, 0, 0) != SQLITE_OK)
//     {
//         printf("Failed to insert data: %s\n", sqlite3_errmsg(db));
//     }

//     // Query data
//     query_database(db, select_sql);

//     // Close the database
//     sqlite3_close(db);

//     return 0;
// }

#endif