#include "dbconn.hpp"
#include "json.hpp"
using json = nlohmann::json;

int main() {
    dbConn::Connector dbConn("127.0.0.1", "12345");
    
    // dbConn.connectQueryClose("CREATE TABLE hangTable (id INT, word VARCHAR(255));");
    // dbConn.connectQueryClose("INSERT INTO hangTable (id, word) VALUES (1, 'purple'), (2, 'peanut');");
    json response = dbConn.connectQueryClose("SELECT * FROM hangTable WHERE id = 2;");

    dbConn::Connector::printResponse(response);

    return 0;

}

/*
    "CREATE TABLE testTable (id INT, isDeleted BOOL, message VARCHAR(255));";
    "INSERT INTO testTable (id, isDeleted, message) VALUES (1, false, 'first message'), (2, true, 'second message');"
    "SELECT * FROM testTable;";
*/