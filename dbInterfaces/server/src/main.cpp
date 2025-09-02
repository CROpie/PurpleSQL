#include <iostream>
#include <string>
#include "dbconn.hpp"
#include "json.hpp"
using json = nlohmann::json;

int main() {
    dbConn::Connector dbConn("127.0.0.1", "12345");
    int db_fd;
    
    // dbConn.connectQueryClose("CREATE TABLE hangTable (id INT, word VARCHAR(255));");
    // dbConn.connectQueryClose("INSERT INTO hangTable (id, word) VALUES (1, 'purple'), (2, 'peanut');");

    db_fd = dbConn.connectToDb();

    while (true) {
        std::string sql;
        std::cout << "db > ";
        std::getline(std::cin, sql);

        if (sql == "quit") {
            break;
        }

        // std::cout << sql << "\n";
        json response = dbConn.queryDB(db_fd, sql.c_str());

        std::cout << response.dump(4) << std::endl;
    }

    dbConn.closeConnection(db_fd);

    return 0;

}

/*
    "CREATE TABLE testTable (id INT, isDeleted BOOL, message VARCHAR(255));";
    "INSERT INTO testTable (id, isDeleted, message) VALUES (1, false, 'first message'), (2, true, 'second message');"
    "SELECT * FROM testTable;";


    CREATE TABLE peanutTable (id INT, word VARCHAR(255));
    INSERT INTO peanutTable (id, word) VALUES (1, 'purple'), (2, 'peanut');

*/